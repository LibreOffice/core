using System.Globalization;
using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Graphics;
using Paperless.Core.Numbering;
using Paperless.Text.Encodings;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Rtf;

/// <summary>
/// Reads an RTF document into the format-independent content tree.
/// </summary>
/// <remarks>
/// <para>
/// RTF is a stream of nested groups rather than a tree of elements, so this is a state machine
/// rather than a walk: a group's opening brace saves the formatting state and its closing brace
/// restores it, and every control word either changes that state or emits something.
/// </para>
/// <para>
/// Three things make RTF unlike the XML formats, and each shapes the code below:
/// </para>
/// <para>
/// <strong>The encoding is decided by the document, mid-stream.</strong> <c>\ansicpg</c> names a
/// code page for <c>\'hh</c> escapes and literal bytes, and a font's <c>\fcharset</c> can
/// override it. So text is tokenised as bytes and decoded here, where what is in force is known.
/// </para>
/// <para>
/// <strong>An unknown destination must be skipped whole.</strong> <c>\*</c> marks a group whose
/// content is meaningless to a reader that does not know the destination — and RTF puts real
/// content and private extensions in the same syntax, so recursing hopefully into everything
/// emits binary picture data and revision bookkeeping as text.
/// </para>
/// <para>
/// <strong>The list label is written out.</strong> Unlike ODF and DOCX, which record only the
/// counters, RTF carries the rendered label in a <c>{\listtext}</c> group — so the marker is read
/// rather than generated, which is both simpler and exactly what the writer displayed.
/// </para>
/// </remarks>
public sealed partial class RtfDocumentReader
{
    /// <summary>
    /// How deeply groups are followed before the reader gives up.
    /// </summary>
    /// <remarks>
    /// Real documents nest perhaps twenty deep. A corrupt or hostile file can open braces
    /// indefinitely, and this is untrusted input driving a stack.
    /// </remarks>
    public const int MaxGroupDepth = 256;

    /// <summary>How many paragraphs are recorded for layout before the rest are ignored.</summary>
    /// <remarks>
    /// A guard on untrusted input rather than a real limit, and it bounds memory rather than time: the
    /// content walk continues either way, so a document past the cap still extracts in full.
    /// </remarks>
    public const int MaxLayoutParagraphs = 200000;

    /// <summary>How many tab stops a paragraph may declare before the rest are ignored.</summary>
    /// <remarks>
    /// A guard on untrusted input. Word's own limit is 64 and a real paragraph uses a handful; a generated
    /// file can restate <c>\tx</c> without end, and each stop costs a lookup on every tab.
    /// </remarks>
    public const int MaxTabStops = 256;

    /// <summary>How many colour table entries are kept.</summary>
    /// <remarks>
    /// A guard on untrusted input rather than a real limit: a document uses a handful, and \cf indexes
    /// past the table resolve to no colour anyway.
    /// </remarks>
    public const int MaxColours = 4096;

    private readonly byte[] _data;
    private readonly List<Diagnostic> _diagnostics;
    private readonly RtfStyles _styles = new();
    private readonly Dictionary<int, int> _fontCharsets = [];
    private readonly Dictionary<int, string> _fontFamilies = [];

    /// <summary>
    /// The colour table, in declaration order, zero-based as <c>\cf</c> indexes it.
    /// </summary>
    /// <remarks>
    /// An entry is null when it stated no components at all, which is the "automatic" colour rather than
    /// black: a <c>\colortbl</c> conventionally opens with a bare semicolon for exactly that, and
    /// LibreOffice's own export writes one. Dropping the empty entry instead of storing it would shift
    /// every colour in the document by one — which is a silver word where a red one belongs.
    /// </remarks>
    private readonly List<Colour?> _colours = [];
    private readonly Staged _layoutBlocks = new();
    private readonly Dictionary<(int Section, Model.PageFurnitureSlot Slot), List<RtfLayoutParagraph>>
        _headerLayout = [];
    private readonly Dictionary<(int Section, Model.PageFurnitureSlot Slot), List<RtfLayoutParagraph>>
        _footerLayout = [];
    private readonly List<Flow> _flows = [];
    private readonly List<ContentNode> _hoisted = [];
    private readonly Dictionary<string, string> _info = new(StringComparer.Ordinal);
    private readonly RtfPageGeometry _geometry = new();

    private Encoding _documentEncoding = LegacyCodePages.Fallback;
    /// <summary>
    /// The document's <c>\deftab</c>, or Word's own default.
    /// </summary>
    /// <remarks>
    /// Half an inch, because RTF is Word's format and that is what Word uses — unlike ODF, whose default is
    /// 1.25 cm. A document that states none and relies on the wrong one puts every tabulated column in the
    /// wrong place.
    /// </remarks>
    private Core.Units.Length _defaultTabInterval = Core.Units.Length.FromTwips(720);

    private int _colourRed;
    private int _colourGreen;
    private int _colourBlue;
    private bool _colourStated;
    private int _footnoteNumber;
    private int _footnoteStart = 1;
    private string? _pendingAnnotationAuthor;
    private bool _reportedDepthLimit;
    private bool _reportedEncoding;

    /// <summary>Creates a reader over a whole RTF document.</summary>
    public RtfDocumentReader(byte[] data, List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(data);
        ArgumentNullException.ThrowIfNull(diagnostics);
        _data = data;
        _diagnostics = diagnostics;
    }

    /// <summary>The metadata read from the document's <c>{\info}</c> group.</summary>
    public DocumentMetadata Metadata { get; private set; } = DocumentMetadata.Empty;

    /// <summary>The sections' page geometry, valid once <see cref="Read"/> has run.</summary>
    public IReadOnlyList<Model.WritingSection> Sections => _geometry.Sections;

    /// <summary>
    /// The body's blocks — its paragraphs and its tables — with the formatting layout needs, valid once
    /// <see cref="Read"/> has run.
    /// </summary>
    /// <remarks>
    /// Collected during the content walk rather than by a second pass, unlike the XML formats. RTF is a
    /// token stream with no structure to revisit — re-reading it would mean running the whole state
    /// machine again, including its encoding and destination handling, and the two runs could then
    /// disagree. So the formatting in force is recorded as each paragraph closes.
    /// </remarks>
    public IReadOnlyList<RtfLayoutBlock> LayoutBlocks => _layoutBlocks.Finished();

    /// <summary>The headers' paragraphs, by section and slot, with the formatting layout needs.</summary>
    /// <remarks>
    /// Separate from <see cref="LayoutBlocks"/> rather than mixed in with a marker, because a header is
    /// laid out into its own frame and the body's paragraph index has to keep meaning the body's. Keyed by
    /// section as well as slot because RTF writes a header in the preamble of the section it belongs to, so
    /// a document that changes its running head halfway says so by writing a second one.
    /// </remarks>
    public IReadOnlyDictionary<(int Section, Model.PageFurnitureSlot Slot), List<RtfLayoutParagraph>>
        HeaderLayout => _headerLayout;

    /// <summary>The footers' paragraphs, by section and slot.</summary>
    public IReadOnlyDictionary<(int Section, Model.PageFurnitureSlot Slot), List<RtfLayoutParagraph>>
        FooterLayout => _footerLayout;

    /// <summary>
    /// How many <c>\sect</c> marks have been seen, which is the section the reader is in.
    /// </summary>
    /// <remarks>
    /// RTF delimits sections by position and marks only their <em>ends</em>: <c>\sect</c> closes one, and
    /// the last section of a document has none at all — it simply runs out. So counting the marks is the
    /// whole of it, and a document with no <c>\sect</c> is one section, which is most of them.
    /// </remarks>
    private int _sectionIndex;

    /// <summary>Reads the document.</summary>
    public ContentDocument Read()
    {
        ContentSection body = new() { Kind = SectionKind.Body, Index = 0 };
        _flows.Add(new Flow(body));

        RtfTokeniser tokeniser = new(_data);
        List<GroupState> stack = [new GroupState()];
        bool nextIsIgnorableDestination = false;

        while (true)
        {
            RtfToken token = tokeniser.Read();
            if (token.Kind == RtfTokenKind.End) break;

            GroupState state = stack[^1];

            switch (token.Kind)
            {
                case RtfTokenKind.GroupStart:
                    _groupDepth = stack.Count;
                    if (stack.Count >= MaxGroupDepth)
                    {
                        ReportDepthLimit();
                        // Still push a skipping state, so the matching brace pops correctly and
                        // the rest of the document is not thrown away.
                        stack.Add(new GroupState { Destination = RtfDestination.Skip });
                        break;
                    }
                    stack.Add(state.Clone());
                    nextIsIgnorableDestination = false;
                    break;

                case RtfTokenKind.GroupEnd:
                    _groupDepth = stack.Count - 1;
                    CloseGroup(stack[^1]);
                    if (stack.Count > 1) stack.RemoveAt(stack.Count - 1);
                    break;

                case RtfTokenKind.ControlSymbol:
                    if (token.Symbol == '*')
                    {
                        nextIsIgnorableDestination = true;
                        break;
                    }
                    HandleControlSymbol(token.Symbol, state);
                    break;

                case RtfTokenKind.ControlWord:
                    HandleControlWord(token, state, ref nextIsIgnorableDestination);
                    break;

                case RtfTokenKind.UnicodeCharacter:
                    AppendText(state, char.ConvertFromUtf32(token.CodePoint));
                    // \uN is followed by however many fallback characters \uc says, which
                    // duplicate the same character in the document's code page. Emitting them
                    // would double every non-ASCII character.
                    SkipUnicodeFallback(tokeniser, state.UnicodeSkip);
                    break;

                case RtfTokenKind.EscapedByte:
                    AppendText(state, DecodeByte((byte)token.CodePoint, state));
                    break;

                case RtfTokenKind.Text:
                    AppendText(state, Decode(token.Bytes.Span, state));
                    break;
            }
        }

        // A document need not end its last paragraph with \par, and the state in force at the end of
        // the stream is the one that would have applied to it.
        FinishParagraph(_flows[0], stack[^1]);
        CloseTablesDeeperThan(_flows[0], 0);

        ContentDocument document = new() { Metadata = BuildMetadata() };
        document.Children.Add(body);
        foreach (ContentNode node in _hoisted) document.Children.Add(node);
        return document;
    }

    // -------------------------------------------------------------------- control words

    private void HandleControlWord(RtfToken token, GroupState state, ref bool ignorableDestination)
    {
        bool ignorable = ignorableDestination;
        ignorableDestination = false;

        // Page geometry is thirty control words that all do the same thing, so it is accumulated
        // elsewhere and consulted first. Only \sect and \sectd are also structural, and those fall
        // through to the break handling below.
        if (_geometry.Handle(token.Name, token.Parameter)
            && token.Name is not ("sect" or "sectd"))
        {
            return;
        }

        switch (token.Name)
        {
            // ---- document-level settings
            case "ansicpg":
                SetDocumentEncoding(token.Parameter ?? LegacyCodePages.WindowsWestern);
                return;
            case "mac":
                SetDocumentEncoding(10000);
                return;
            case "pc":
                SetDocumentEncoding(437);
                return;
            case "pca":
                SetDocumentEncoding(850);
                return;
            case "ansi":
                SetDocumentEncoding(LegacyCodePages.WindowsWestern);
                return;
            case "uc":
                state.UnicodeSkip = Math.Clamp(token.Parameter ?? 1, 0, 32);
                return;
            case "ftnstart":
                _footnoteStart = token.Parameter ?? 1;
                return;

            // ---- destinations that are not content
            case "fonttbl":
                state.Destination = RtfDestination.FontTable;
                return;
            case "colortbl":
                // Read rather than skipped, because a run's colour is the one piece of character
                // formatting that reaches a page without changing where anything sits.
                state.Destination = RtfDestination.ColourTable;
                BeginColourTable();
                return;
            case "listtable" or "listoverridetable" or "revtbl" or "rsidtbl"
                 or "generator" or "filetbl" or "themedata" or "colorschememapping"
                 or "datastore" or "latentstyles" or "xmlnstbl" or "pgptbl":
                state.Destination = token.Name == "generator"
                    ? RtfDestination.InfoField
                    : RtfDestination.Skip;
                if (token.Name == "generator") state.InfoField = "generator";
                return;
            case "stylesheet":
                state.Destination = RtfDestination.StyleSheet;
                return;
            case "info":
                state.Destination = RtfDestination.Info;
                return;
            case "pict":
                state.Destination = RtfDestination.Picture;
                return;
            case "object" or "objdata" or "result":
                // An embedded object: its data is binary and its \result is a rendering of
                // something Paperless does not open during extraction.
                state.Destination = RtfDestination.Skip;
                return;
            case "bkmkstart" or "bkmkend" or "atnid" or "atnref" or "atndate" or "atnparent"
                 or "annotprot" or "xe" or "tc" or "tcn" or "datafield" or "fname" or "ftnsep"
                 or "ftnsepc" or "ftncn" or "aftnsep" or "aftnsepc" or "aftncn" or "nonshppict"
                 or "mmath" or "do" or "shpinst" or "shprslt" or "svb" or "template"
                 or "keycode" or "password" or "passwordhash" or "protend" or "protstart":
                state.Destination = RtfDestination.Skip;
                return;
            case "atnauthor":
                state.Destination = RtfDestination.AnnotationAuthor;
                return;

            // ---- flows
            case "footnote":
                // The reference mark was written just before this group, so the number it took is
                // the one this note carries.
                BeginFlow(state, SectionKind.Note, CurrentFootnoteCitation());
                return;
            case "annotation":
                BeginFlow(state, SectionKind.Comment, _pendingAnnotationAuthor);
                _pendingAnnotationAuthor = null;
                return;
            case "header" or "headerl" or "headerr" or "headerf":
                BeginFlow(state, SectionKind.Header, token.Name[6..] is "" ? "default" : token.Name[6..]);
                return;
            case "footer" or "footerl" or "footerr" or "footerf":
                BeginFlow(state, SectionKind.Footer, token.Name[6..] is "" ? "default" : token.Name[6..]);
                return;
            case "shptxt" or "txbxtext":
                // A shape's or text box's own text flow, which is not part of the paragraph it
                // is anchored in.
                BeginFlow(state, SectionKind.Frame, null);
                return;

            // ---- fields
            case "field":
                // A field's hyperlink applies only inside it, so the depth it was seen at is what
                // says when to forget it.
                _fieldHyperlink = null;
                _fieldDepth = _groupDepth;
                return;
            case "fldinst":
                state.Destination = RtfDestination.FieldInstruction;
                return;
            case "fldrslt":
                // The cached result, which is what a reader displays.
                state.Destination = RtfDestination.Body;
                return;

            // ---- list labels
            case "listtext":
                state.Destination = RtfDestination.ListText;
                return;
            case "ls":
                state.ListId = token.Parameter ?? 0;
                return;
            case "ilvl":
                state.ListLevel = token.Parameter ?? 0;
                return;

            // ---- layout formatting
            case "fs":
                // Half-points, as in OOXML: \fs24 is twelve points.
                state.FontSizeHalfPoints = token.Parameter;
                return;
            // ---- tab stops. The kind and the leader precede the position, and \pard clears the lot.
            case "tqc":
                state.PendingTabAlignment = TabAlignment.Centre;
                return;
            case "tqr":
                state.PendingTabAlignment = TabAlignment.Right;
                return;
            case "tqdec":
                state.PendingTabAlignment = TabAlignment.DecimalSeparator;
                return;
            case "tldot":
                state.PendingTabLeader = '.';
                return;
            case "tlhyph":
                state.PendingTabLeader = '-';
                return;
            case "tlul" or "tlth":
                state.PendingTabLeader = '_';
                return;
            case "tleq":
                state.PendingTabLeader = '=';
                return;
            case "tx" or "tb":
                // \tb is a bar tab: a vertical rule rather than an advance, so it is recorded as a plain
                // stop. Drawing the rule needs the line's height, which a paragraph property does not have.
                if (token.Parameter is { } position && state.TabStops.Count < MaxTabStops)
                {
                    state.TabStops.Add(new TabStop(
                        Core.Units.Length.FromTwips(position),
                        state.PendingTabAlignment,
                        state.PendingTabLeader));
                }

                state.PendingTabAlignment = TabAlignment.Left;
                state.PendingTabLeader = '\0';
                return;
            case "deftab":
                if (token.Parameter is { } interval and > 0)
                    _defaultTabInterval = Core.Units.Length.FromTwips(interval);
                return;

            case "li":
                state.LeftIndent = token.Parameter;
                return;
            case "ri":
                state.RightIndent = token.Parameter;
                return;
            case "fi":
                state.FirstLineIndent = token.Parameter;
                return;
            case "sb":
                state.SpaceBefore = token.Parameter;
                return;
            case "sa":
                state.SpaceAfter = token.Parameter;
                return;
            case "sl":
                state.LineSpacing = token.Parameter;
                return;
            case "slmult":
                // \slmult1 makes \sl a multiple in two-hundred-and-fortieths of a line; \slmult0 or
                // its absence makes it twips, whose sign then chooses at-least from exact.
                state.IsMultipleLineSpacing = token.Parameter is not 0;
                return;
            case "ql":
                state.Alignment = TextAlignment.Start;
                return;
            case "qr":
                state.Alignment = TextAlignment.End;
                return;
            case "qc":
                state.Alignment = TextAlignment.Centre;
                return;
            case "qj":
                state.Alignment = TextAlignment.Justify;
                return;
            case "qd":
                state.Alignment = TextAlignment.Distribute;
                return;
            case "keepn":
                state.KeepWithNext = token.Parameter is not 0;
                return;
            case "keep":
                state.KeepTogether = token.Parameter is not 0;
                return;
            case "widctlpar":
                state.HasWidowControl = true;
                return;
            case "nowidctlpar":
                state.HasWidowControl = false;
                return;
            case "pagebb":
                state.StartsNewPage = token.Parameter is not 0;
                return;
            case "contextualspace":
                state.HasContextualSpacing = token.Parameter is not 0;
                return;

            // ---- paragraph and character state
            case "pard":
                state.ResetParagraph();
                // Table membership is paragraph formatting, so \pard clears it and the \intbl and
                // \itap that follow re-state it. Without the reset a paragraph after a table stays
                // in it, and the table never closes.
                CurrentFlow.InTable = false;
                CurrentFlow.TableLevelIndex = 0;
                return;
            case "plain":
                state.ResetCharacter();
                return;
            case "par":
                EmitParagraph(state);
                return;
            case "line":
                AppendText(state, "\n");
                return;
            case "tab":
                AppendText(state, "\t");
                return;
            case "s":
                if (state.Destination == RtfDestination.StyleSheet)
                {
                    // Inside the stylesheet, \s names the style being *defined* rather than one
                    // being applied. Missing this leaves every style nameless.
                    state.StyleSheetId = token.Parameter ?? 0;
                    state.StyleSheetIsCharacter = false;
                }
                else
                {
                    state.ParagraphStyleId = token.Parameter ?? 0;
                }
                return;
            case "cs":
                if (state.Destination == RtfDestination.StyleSheet)
                {
                    state.StyleSheetId = token.Parameter ?? 0;
                    state.StyleSheetIsCharacter = true;
                }
                else
                {
                    state.CharacterStyleId = token.Parameter ?? 0;
                }
                return;
            case "outlinelevel":
                state.OutlineLevel = token.Parameter;
                return;
            case "sbasedon":
                state.StyleSheetBasedOn = token.Parameter;
                return;
            case "b":
                state.Bold = token.Parameter != 0;
                return;
            case "i":
                state.Italic = token.Parameter != 0;
                return;
            case "ul" or "uld" or "uldash" or "uldashd" or "uldashdd" or "uldb" or "ulhwave"
                 or "ulldash" or "ulth" or "ulthd" or "ulthdash" or "ulthdashd" or "ulthdashdd"
                 or "ulthldash" or "ululdbwave" or "ulw" or "ulwave":
                state.Underline = token.Parameter != 0;
                return;
            case "ulnone":
                state.Underline = false;
                return;
            case "strike" or "striked":
                state.Strike = token.Parameter != 0;
                return;
            case "super":
                state.VerticalPosition = token.Parameter == 0 ? 0 : 1;
                return;
            case "sub":
                state.VerticalPosition = token.Parameter == 0 ? 0 : -1;
                return;
            case "nosupersub":
                state.VerticalPosition = 0;
                return;
            case "v":
                // Hidden text: not displayed by any reader, so extracting it would inject text
                // the document does not show.
                state.Hidden = token.Parameter != 0;
                return;
            case "f":
                state.FontIndex = token.Parameter ?? 0;
                if (state.Destination == RtfDestination.FontTable)
                    state.FontTableIndex = token.Parameter ?? 0;
                return;
            case "fcharset":
                if (state.Destination == RtfDestination.FontTable && state.FontTableIndex is { } font)
                    _fontCharsets[font] = token.Parameter ?? 0;
                return;
            case "lang" or "langnp":
                state.LanguageId = token.Parameter ?? 0;
                return;

            // ---- colours
            case "red" or "green" or "blue":
                if (state.Destination == RtfDestination.ColourTable)
                    SetColourComponent(token.Name, token.Parameter ?? 0);
                return;
            case "cf":
                state.ForegroundColourIndex = token.Parameter is { } index and >= 0 ? index : null;
                return;

            // ---- tables
            case "trowd":
                BeginRowDefinition(CurrentFlow);
                return;
            case "intbl":
                state.InTable = true;
                CurrentFlow.InTable = true;
                return;
            case "itap":
                // The paragraph's table nesting depth. Zero means it is not in a table at all, which
                // is how a producer says "this paragraph left the table" without a closing marker.
                CurrentFlow.TableLevelIndex = Math.Clamp(token.Parameter ?? 0, 0, MaxTableDepth);
                if (CurrentFlow.TableLevelIndex > 0) CurrentFlow.InTable = true;
                return;
            case "trhdr":
                DefinitionTarget(CurrentFlow).RowIsHeader = true;
                return;
            case "cellx":
                AddCellDefinition(CurrentFlow, token.Parameter);
                return;
            case "trleft":
                DefinitionTarget(CurrentFlow).RowLeftEdge = token.Parameter ?? 0;
                return;
            case "trgaph":
                // Half the gap between two cells, so it is the padding on each side of one. RTF's oldest
                // spelling of cell padding and the one LibreOffice writes.
                DefinitionTarget(CurrentFlow).RowHalfGap = token.Parameter ?? 0;
                return;
            case "trrh":
                // A row height. Negative means "exactly this", which is not modelled — the magnitude is
                // taken as a floor either way, which shows the text rather than clipping it.
                DefinitionTarget(CurrentFlow).RowHeight = Math.Abs(token.Parameter ?? 0);
                return;
            case "clpadl" or "clpadr" or "clpadt" or "clpadb":
                SetPadding(
                    DefinitionTarget(CurrentFlow).PendingCellPadding,
                    CellPaddingSide(token.Name[5]),
                    token.Parameter);
                return;
            case "trpaddl" or "trpaddr" or "trpaddt" or "trpaddb":
                SetPadding(
                    DefinitionTarget(CurrentFlow).RowPadding,
                    RowPaddingSide(token.Name[6]),
                    token.Parameter);
                return;
            case "trpaddfl" or "trpaddfr" or "trpaddft" or "trpaddfb":
                // The unit flag for the matching \trpadd, as \clpadf* is for \clpad*.
                return;
            case "clpadfl" or "clpadfr" or "clpadft" or "clpadfb":
                // The unit the matching \clpad takes: 3 is twips and 0 is a "null" the specification
                // leaves undefined. Only twips is honoured, which is what every producer writes.
                return;
            case "clvertalt":
                DefinitionTarget(CurrentFlow).PendingCellAlignment = Layout.CellVerticalAlignment.Top;
                return;
            case "clvertalc":
                DefinitionTarget(CurrentFlow).PendingCellAlignment = Layout.CellVerticalAlignment.Middle;
                return;
            case "clvertalb":
                DefinitionTarget(CurrentFlow).PendingCellAlignment = Layout.CellVerticalAlignment.Bottom;
                return;
            case "clmgf":
                DefinitionTarget(CurrentFlow).PendingCellMergesFirst = true;
                return;
            case "clmrg":
                DefinitionTarget(CurrentFlow).PendingCellMerged = true;
                return;
            case "clvmgf":
                DefinitionTarget(CurrentFlow).PendingCellVerticalFirst = true;
                return;
            case "clvmrg":
                DefinitionTarget(CurrentFlow).PendingCellVerticalMerged = true;
                return;
            case "nesttableprops":
                // A nested row's definition, and the one ignorable destination that must not be
                // skipped: it holds the \trowd, the \cellx edges and the \nestrow that closes the
                // row. It contains no text, so leaving the destination alone is enough.
                return;
            case "nonesttables":
                // A plain-text approximation of the nested table, for readers that cannot nest.
                // Reading it as well as the real thing duplicates every nested cell.
                state.Destination = RtfDestination.Skip;
                return;
            case "cell" or "nestcell":
                EndCell(state);
                return;
            case "row" or "nestrow":
                EndRow(state);
                return;

            // ---- characters written as control words
            case "chftn":
                // The auto-numbered footnote mark. RTF stores no number: the reference and the
                // note's own mark are both this control word, and the writer expects the reader
                // to count. The mark inside a note repeats the reference's number rather than
                // taking a new one, which is why only the body advances the counter.
                if (CurrentFlow.Target.Kind == SectionKind.Note)
                {
                    // The mark inside the note's own body repeats the reference's number. The
                    // note section already carries it as its name, so emitting it here would
                    // prefix every note's text with its own number — which is not what the other
                    // readers do and would make the same document extract differently per format.
                    return;
                }
                _footnoteNumber++;
                AppendText(state, CurrentFootnoteCitation());
                return;
            case "chatn" or "chftnsep" or "chftnsepc":
                return;
            case "emdash":
                AppendText(state, "—");
                return;
            case "endash":
                AppendText(state, "–");
                return;
            case "emspace":
                AppendText(state, " ");
                return;
            case "enspace":
                AppendText(state, " ");
                return;
            case "qmspace":
                AppendText(state, " ");
                return;
            case "bullet":
                AppendText(state, "•");
                return;
            case "lquote":
                AppendText(state, "‘");
                return;
            case "rquote":
                AppendText(state, "’");
                return;
            case "ldblquote":
                AppendText(state, "“");
                return;
            case "rdblquote":
                AppendText(state, "”");
                return;
            case "zwnj":
                AppendText(state, "‌");
                return;
            case "zwj":
                AppendText(state, "‍");
                return;
            case "sect":
                // Ends the section, and the paragraph with it. The geometry reader saw this word first and
                // has already recorded the section that closed here, so what follows belongs to the next.
                // Not forced: LibreOffice writes \par before \sect, and forcing would add a blank
                // paragraph at every section boundary — while a producer writing text straight into \sect
                // still gets its last paragraph closed.
                FinishParagraph(CurrentFlow, state, force: false);
                _sectionIndex++;
                return;

            case "page" or "column" or "sectd" or "softpage":
                // Breaks move content elsewhere without contributing text.
                return;
            case "deleted":
                // Text a tracked change removed, which is still in the file.
                state.Destination = RtfDestination.Skip;
                return;

            // ---- info fields
            case "title" or "subject" or "author" or "operator" or "keywords" or "doccomm"
                 or "category" or "manager" or "company":
                if (state.Destination == RtfDestination.Info)
                {
                    state.Destination = RtfDestination.InfoField;
                    state.InfoField = token.Name;
                }
                return;
            case "creatim" or "revtim" or "printim":
                if (state.Destination == RtfDestination.Info)
                {
                    state.Destination = RtfDestination.InfoTimestamp;
                    state.InfoField = token.Name;
                }
                return;
            case "nofpages" or "nofwords" or "nofchars" or "nofcharsws" or "version" or "edmins":
                if (state.Destination is RtfDestination.Info && token.Parameter is { } value)
                    _info[token.Name] = value.ToString(CultureInfo.InvariantCulture);
                return;
            case "yr" or "mo" or "dy" or "hr" or "min" or "sec":
                if (state.Destination == RtfDestination.InfoTimestamp && state.InfoField is { } field
                    && token.Parameter is { } part)
                    _info[$"{field}.{token.Name}"] = part.ToString(CultureInfo.InvariantCulture);
                return;

            default:
                // An unknown destination marked with \* must be skipped whole: RTF puts private
                // extensions in the same syntax as content, and reading one emits its internals
                // as text.
                if (ignorable) state.Destination = RtfDestination.Skip;
                return;
        }
    }

    private void HandleControlSymbol(char symbol, GroupState state)
    {
        switch (symbol)
        {
            case '\\' or '{' or '}':
                AppendText(state, symbol.ToString());
                break;
            case '~':
                AppendText(state, " ");
                break;
            case '_':
                AppendText(state, "‑");
                break;
            case '-':
                // A discretionary hyphen is drawn only where the line breaks, so it contributes
                // nothing to the text.
                break;
            default:
                break;
        }
    }

    // ------------------------------------------------------------------------ encoding

    private void SetDocumentEncoding(int codePage)
    {
        _documentEncoding = LegacyCodePages.Get(codePage, out bool resolved);
        if (!resolved && !_reportedEncoding)
        {
            _reportedEncoding = true;
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2200",
                $"The document declares code page {codePage}, which is not available; text has "
                + $"been decoded as {_documentEncoding.WebName}. Non-ASCII characters may be "
                + "wrong."));
        }
    }

    /// <summary>
    /// The encoding for text in the current state: the font's character set where it names one,
    /// otherwise the document's.
    /// </summary>
    /// <remarks>
    /// A font's <c>\fcharset</c> overrides <c>\ansicpg</c> for text in that font, which is how a
    /// single RTF file mixes Cyrillic and Western text. Charset 2 is Symbol and charset 0 is
    /// "ANSI", neither of which changes the code page.
    /// </remarks>
    private Encoding EncodingFor(GroupState state)
    {
        if (!_fontCharsets.TryGetValue(state.FontIndex, out int charset)) return _documentEncoding;

        return charset switch
        {
            0 or 1 or 2 => _documentEncoding,
            77 => LegacyCodePages.Get(10000),   // Mac Roman
            128 => LegacyCodePages.Get(932),    // Shift-JIS
            129 => LegacyCodePages.Get(949),    // Wansung
            130 => LegacyCodePages.Get(1361),   // Johab
            134 => LegacyCodePages.Get(936),    // GBK
            136 => LegacyCodePages.Get(950),    // Big5
            161 => LegacyCodePages.Get(1253),   // Greek
            162 => LegacyCodePages.Get(1254),   // Turkish
            163 => LegacyCodePages.Get(1258),   // Vietnamese
            177 => LegacyCodePages.Get(1255),   // Hebrew
            178 or 179 or 180 => LegacyCodePages.Get(1256), // Arabic
            186 => LegacyCodePages.Get(1257),   // Baltic
            204 => LegacyCodePages.Get(1251),   // Cyrillic
            222 => LegacyCodePages.Get(874),    // Thai
            238 => LegacyCodePages.Get(1250),   // Central European
            _ => _documentEncoding,
        };
    }

    private string Decode(ReadOnlySpan<byte> bytes, GroupState state)
        => bytes.IsEmpty ? string.Empty : EncodingFor(state).GetString(bytes);

    private string DecodeByte(byte value, GroupState state)
    {
        Span<byte> single = [value];
        return EncodingFor(state).GetString(single);
    }

    /// <summary>
    /// Consumes the code-page fallback characters that follow a <c>\uN</c>.
    /// </summary>
    /// <remarks>
    /// A writer emits both a Unicode code point and its best approximation in the document's code
    /// page, so that older readers see something. <c>\ucN</c> says how many characters the
    /// approximation takes. Not skipping them doubles every non-ASCII character — with the
    /// duplicate usually rendered as a question mark, which is what makes the symptom look like
    /// an encoding bug rather than a parsing one.
    /// </remarks>
    private static void SkipUnicodeFallback(RtfTokeniser tokeniser, int count)
    {
        for (int skipped = 0; skipped < count;)
        {
            RtfToken token = tokeniser.Read();
            switch (token.Kind)
            {
                case RtfTokenKind.End:
                    return;
                case RtfTokenKind.EscapedByte:
                    skipped++;
                    break;
                case RtfTokenKind.Text:
                    // A text token may cover several fallback characters at once.
                    skipped += token.Bytes.Length;
                    break;
                default:
                    // A group boundary or control word ends the fallback run: the approximation
                    // is plain characters only.
                    return;
            }
        }
    }

    private void ReportDepthLimit()
    {
        if (_reportedDepthLimit) return;
        _reportedDepthLimit = true;
        _diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Error, "PL2201",
            $"Groups are nested more than {MaxGroupDepth} deep; the deeper content has been "
            + "skipped."));
    }

    private string CurrentFootnoteCitation()
        => OutlineNumbers.Digits(_footnoteStart + Math.Max(0, _footnoteNumber - 1));

    private DocumentMetadata BuildMetadata()
    {
        Metadata = new DocumentMetadata
        {
            Title = Info("title"),
            Subject = Info("subject"),
            Author = Info("author"),
            LastModifiedBy = Info("operator"),
            Description = Info("doccomm"),
            Category = Info("category"),
            GeneratorApplication = Info("generator"),
            Keywords = Info("keywords") is { } keywords
                ? [.. keywords.Split([',', ';'], StringSplitOptions.RemoveEmptyEntries
                                                 | StringSplitOptions.TrimEntries)]
                : [],
            Created = Timestamp("creatim"),
            Modified = Timestamp("revtim"),
            Printed = Timestamp("printim"),
            RevisionNumber = Number("version"),
            // \edmins counts minutes of editing time, not a duration string.
            TotalEditingTime = Number("edmins") is { } minutes ? TimeSpan.FromMinutes(minutes) : null,
            Statistics = _info.Keys.Any(k => k is "nofpages" or "nofwords" or "nofchars")
                ? new DocumentStatistics
                {
                    PageCount = Number("nofpages"),
                    WordCount = Number("nofwords"),
                    // \nofcharsws includes spaces, which is the count a person means.
                    CharacterCount = Number("nofcharsws") ?? Number("nofchars"),
                }
                : null,
        };
        return Metadata;

        string? Info(string name)
            => _info.TryGetValue(name, out string? value) && value.Trim().Length > 0
                ? value.Trim()
                : null;

        int? Number(string name)
            => Info(name) is { } text
               && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int parsed)
                ? parsed
                : null;

        DateTimeOffset? Timestamp(string prefix)
        {
            if (!_info.TryGetValue($"{prefix}.yr", out string? year)) return null;

            int Part(string name, int fallback)
                => _info.TryGetValue($"{prefix}.{name}", out string? value)
                   && int.TryParse(value, out int parsed)
                    ? parsed
                    : fallback;

            try
            {
                // RTF timestamps carry no zone, so the same rule as ODF applies: report a zero
                // offset rather than shifting by the reading machine's zone, which would make one
                // file report different times on different machines.
                return new DateTimeOffset(
                    int.Parse(year, CultureInfo.InvariantCulture),
                    Part("mo", 1), Part("dy", 1), Part("hr", 0), Part("min", 0), Part("sec", 0),
                    TimeSpan.Zero);
            }
            catch (ArgumentOutOfRangeException)
            {
                // A malformed date is not a reason to lose the rest of the metadata.
                return null;
            }
            catch (FormatException)
            {
                return null;
            }
        }
    }
}
