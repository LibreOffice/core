using System.Globalization;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml;
using XRay.Office.Ooxml.OfficeMath;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Shaping;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using XRay.Office.Core.Threading;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// Turns a DOCX body into the paragraphs the paginator takes.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over <c>document.xml</c>, for the same reason ODF has one: extraction discards the font
/// sizes, indents and spacing layout needs, and making it carry them would charge every caller for a
/// feature most never use.
/// </para>
/// <para>
/// The same gap as the ODF source: per-run font sizes are not honoured, so a paragraph is measured
/// wholly in the font its paragraph mark carries. The tallest run on a line sets that line's height, so
/// a paragraph mixing sizes lays out slightly short until the runs are walked.
/// </para>
/// </remarks>
public sealed partial class DocxLayoutSource
{
    /// <summary>How many paragraphs are read before the rest are ignored.</summary>
    public const int MaxParagraphs = 200000;

    /// <summary>
    /// The character an anchor occupies: a field result, a note reference, an inline drawing.
    /// </summary>
    /// <remarks>
    /// The same one the document model and the ODF source use, so an offset means the same thing
    /// wherever it was counted.
    /// </remarks>
    private const char AnchorCharacter = '\u0001';

    /// <summary>
    /// The character a <c>w:br</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, whose UAX #14 class is a mandatory break, so the break iterator honours it without layout
    /// special-casing anything. A newline would break the same way but would read as the end of a
    /// paragraph to anything that later scans the text, which a break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    private readonly WordStyles _styles;
    private readonly SystemFontResolver _fonts;
    private readonly Length _defaultTabInterval;
    private readonly int _compatibilityMode;

    /// <summary>What <c>w:beforeAutospacing</c> and <c>w:afterAutospacing</c> stand for here.</summary>
    private readonly Length _autoSpacing;

    /// <summary>The device grid every font metric is rounded onto.</summary>
    /// <remarks>
    /// Always one or the other, never nothing: <see cref="MetricGrid.Reference"/> is the 8640 dpi
    /// virtual device Writer formats against by default. <c>w:usePrinterMetrics</c> swaps it for a real
    /// printer's — writerfilter turns that flag into <c>PrinterIndependentLayout::DISABLED</c> at
    /// <c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10173</c>, the same state
    /// <c>WW8Dop::fUsePrinterMetrics</c> puts a DOC into, and the same printer grid
    /// <see cref="Ww8.DocReader"/> already passes.
    /// </remarks>
    private readonly MetricGrid _metrics;

    /// <summary>
    /// <c>word/fontTable.xml</c>, for the shape it declares for each family it names.
    /// </summary>
    /// <remarks>
    /// Layout does not measure with it and never asks it for a face. What it settles is the one
    /// question the family name alone cannot answer — whether a family nobody has installed is a
    /// roman or a grotesque — and getting that wrong renders a serif document in DejaVu Sans where
    /// LibreOffice renders it in DejaVu Serif, which moves every line break in it.
    /// </remarks>
    private readonly WordFontTable _fontTable;

    /// <summary>What a <c>FILENAME</c> or <c>TITLE</c> field evaluates to.</summary>
    private readonly ConstantFields _constants;

    private readonly DrawingTheme? _theme;
    private readonly DrawingStyleMatrix? _shapeStyles;
    private readonly Dictionary<(string? Family, int Weight, bool Italic, FontFamilyClass Class,
        WriterScript Script, string? Language), OpenTypeFace?> _faces = [];
    private readonly Dictionary<(string? Family, int Weight, bool Italic, FontFamilyClass Class,
        WriterScript Script, string? Language), FontReference> _references = [];

    /// <summary>Creates a source over a document's styles and settings.</summary>
    /// <param name="styles">The document's styles, including its <c>w:docDefaults</c>.</param>
    /// <param name="settings">The document's <c>w:settings</c> root, or null.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    /// <param name="footnotes">The footnote bodies by <c>w:id</c>, or null for a document with none.</param>
    /// <param name="endnotes">The endnote bodies by <c>w:id</c>.</param>
    /// <param name="theme">The document's theme, for themed run colours, or null.</param>
    /// <param name="shapeStyles">
    /// The theme's <c>a:fmtScheme</c>, or null. An anchored shape stating no fill and no line
    /// width takes both from it — see <c>DocxFrames.Appearance</c> — so a caller that omits it
    /// gets the flowchart boxes and Gantt bars of a themed template drawn as empty outlines.
    /// </param>
    /// <param name="pictures">
    /// How to reach the bytes an <c>a:blip</c> names, or null to lay the document out with its picture
    /// frames empty — which is what a caller who wants only measurements should pay for.
    /// </param>
    /// <param name="numbering">
    /// The document's <c>numbering.xml</c>, or null for a document with no lists. Its counters are
    /// advanced by this walk, which is why <see cref="Read"/> and <see cref="ReadFlow"/> reset them: a
    /// caller sharing one instance with the extraction pass must not have the two interleave.
    /// </param>
    /// <param name="fontTable">
    /// <c>word/fontTable.xml</c>, or null for a document without one. Nothing is measured with it —
    /// it settles which shape a family nobody has installed falls back to, which the family name on
    /// its own cannot say.
    /// </param>
    /// <param name="constants">
    /// What a <c>FILENAME</c> or <c>TITLE</c> field evaluates to, or null to leave both at their
    /// cached results — see <see cref="ConstantFields"/>.
    /// </param>
    public DocxLayoutSource(
        WordStyles styles,
        XElement? settings = null,
        SystemFontResolver? fonts = null,
        IReadOnlyDictionary<string, XElement>? footnotes = null,
        IReadOnlyDictionary<string, XElement>? endnotes = null,
        DrawingTheme? theme = null,
        DrawingStyleMatrix? shapeStyles = null,
        DocxPictures? pictures = null,
        WordNumbering? numbering = null,
        WordFontTable? fontTable = null,
        ConstantFields? constants = null)
    {
        _constants = constants ?? default;
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _fontTable = fontTable ?? WordFontTable.Empty;
        _numbering = numbering ?? new WordNumbering();
        Pictures = pictures;
        _theme = theme;
        _shapeStyles = shapeStyles;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _defaultTabInterval = TabInterval(settings);
        _compatibilityMode = CompatibilityMode(settings);
        WordCompatibility compatibility = WordCompatibility.Read(settings);
        _autoSpacing = AutoSpacing(settings, compatibility);
        // `AsWordDocument` is the MS_WORD_COMP_GRID_METRICS compatibility flag, which the Word
        // filters set and ODF's does not. See MetricGrid.AsWordDocument.
        _metrics = (compatibility.UsesPrinterMetrics ? MetricGrid.Printer : MetricGrid.Reference)
            .AsWordDocument();
        _footnotes = footnotes ?? new Dictionary<string, XElement>(StringComparer.Ordinal);
        _endnotes = endnotes ?? new Dictionary<string, XElement>(StringComparer.Ordinal);
        _footnoteNumbering = NumberingIn(settings, "footnotePr", NoteNumbering.Footnotes);
        _endnoteNumbering = NumberingIn(settings, "endnotePr", NoteNumbering.Endnotes);
    }

    /// <summary>
    /// How a picture's bytes are reached, or null when this source was built without a package.
    /// </summary>
    /// <remarks>
    /// Exposed rather than private because its <see cref="DocxPictures.Scope"/> has to follow the walk:
    /// relationship ids are numbered from one in every part, so whoever hands this source a header to
    /// read must say which part it came from first.
    /// </remarks>
    public DocxPictures? Pictures { get; }

    /// <summary>The footnote bodies by <c>w:id</c>, from <c>footnotes.xml</c>.</summary>
    /// <remarks>
    /// The parts rather than the whole package, because that is all layout needs of it — and passing the
    /// package would let this reach for things the extraction pass owns.
    /// </remarks>
    private readonly IReadOnlyDictionary<string, XElement> _footnotes;

    /// <summary>The endnote bodies by <c>w:id</c>.</summary>
    private readonly IReadOnlyDictionary<string, XElement> _endnotes;

    /// <summary>How the document's footnotes are numbered.</summary>
    private readonly NoteNumbering _footnoteNumbering;

    /// <summary>How its endnotes are numbered, which is a separate sequence in a separate format.</summary>
    private readonly NoteNumbering _endnoteNumbering;

    /// <summary>
    /// The numbering one class of note declares in the document's settings, or the class's default.
    /// </summary>
    /// <remarks>
    /// <c>w:footnotePr</c> and <c>w:endnotePr</c> in <c>w:settings</c>, whose <c>w:numStart</c> is the first
    /// note's number outright — one-based, unlike ODF's <c>text:start-value</c>, which is an offset. A
    /// <em>section</em> can carry the same two elements and override the document's; that is not read, and a
    /// document doing it is numbered by the document-wide values instead.
    /// </remarks>
    /// <param name="settings">The <c>w:settings</c> root, or null.</param>
    /// <param name="element">Which of the two elements to read.</param>
    /// <param name="fallback">The class's default, for whatever the file leaves unsaid.</param>
    private static NoteNumbering NumberingIn(
        XElement? settings, string element, NoteNumbering fallback)
    {
        XElement? properties = Word.Child(settings, element);
        if (properties is null) return fallback;

        NoteNumberFormat format =
            NoteNumbering.Parse(Word.Attribute(Word.Child(properties, "numFmt"), "val"))
            ?? fallback.Format;

        int start = int.TryParse(
            Word.Attribute(Word.Child(properties, "numStart"), "val"),
            NumberStyles.Integer,
            CultureInfo.InvariantCulture,
            out int stated)
            ? stated
            : fallback.StartAt;

        // Where the class collects. `w:pos` means different things for the two elements and only the endnote
        // one matters here: a footnote's `beneathText` is still the foot of its page, while an endnote's
        // `sectEnd` moves it into the note area of the section's last page.
        NotePlacement placement = Word.Attribute(Word.Child(properties, "pos"), "val") switch
        {
            "sectEnd" => NotePlacement.SectionEnd,
            "docEnd" => NotePlacement.DocumentEnd,
            _ => fallback.Placement,
        };

        // Where the count begins again. `eachSect` is not a third kind of restart: Writer has no per-section
        // one, so its chapter restart is what OOXML's `eachSect` both exports from and reads back as.
        NoteRestart restart =
            NoteNumbering.ParseRestart(Word.Attribute(Word.Child(properties, "numRestart"), "val"))
            ?? fallback.Restart;

        return new NoteNumbering(format, start) { Placement = placement, Restart = restart };
    }

    /// <summary>The substitutions made while resolving the document's fonts.</summary>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>
    /// The room Word leaves above a page's notes: the default paragraph style's line height.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>sw::FootnoteSeparatorHeight</c> takes this branch for every document a Word filter opened
    /// (<c>sw/source/core/layout/ftnfrm.cxx</c>:257-272), and its helper is explicit about the source:
    /// <em>"the height of the line that hosts the separator line (the top margin of the container),
    /// based on the default paragraph style"</em>, read as <c>SwFont::GetHeight</c> of that style's
    /// font (<c>:57-77</c>). Not the note's font, and not the body paragraph's — the <em>default
    /// style's</em>, which in a DOCX is what a paragraph naming no <c>w:pStyle</c> resolves to.
    /// </para>
    /// <para>
    /// So it is asked of the same chain and the same faces every other run goes through, and measured
    /// with <see cref="XRay.Office.Text.Fonts.LineSpacing"/> on the document's own device grid: this has
    /// to be the height a line of that text would be, or it is a different number that happens to be
    /// close.
    /// </para>
    /// <para>
    /// Zero when no face can be read at all, which the paginator reads as "no reservation of this
    /// kind"; a document whose default font is missing has larger problems than its note separator.
    /// </para>
    /// </remarks>
    public Length DefaultParagraphLineHeight
    {
        get
        {
            WordTextStyle text = WordParagraphFormats.ResolveText(_styles, null, _theme, fontTable: _fontTable);
            if (Face(text) is not { } face) return Length.Zero;

            return LineSpacing.Resolve(face, _metrics, WriterLineBox.LeadingAboveText)
                .ScaledLineHeight(text.Size);
        }
    }

    /// <summary>
    /// The document's margin line numbering, or null when it asks for none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word states this per section and Writer holds one per document, so the <em>last</em>
    /// <c>w:lnNumType</c> in document order is the one that stands: the importer writes each into the
    /// global settings as it meets it (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:1213 and
    /// :2795), and a section that says nothing does not put the previous one back. A <c>w:countBy</c> of
    /// nought is how a later section turns numbering off, which is the same rule read the other way —
    /// <c>if (aSettings.nInterval == 0) … PROP_IS_ON false</c>.
    /// </para>
    /// <para>
    /// The numbers take the document's <em>default</em> character formatting rather than any paragraph's,
    /// because Writer's <c>Line Numbering</c> character style declares nothing at all; see
    /// <see cref="LineNumbering"/>, where the four probes that establish it are recorded.
    /// </para>
    /// </remarks>
    /// <param name="body">The <c>w:body</c> element.</param>
    public LineNumbering? LineNumbers(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        XElement? stated = body
            .Descendants(Word.Name("sectPr"))
            .Select(section => Word.Child(section, "lnNumType"))
            .LastOrDefault(numbering => numbering is not null);

        if (stated is null) return null;

        if (!Word.Integer(Word.Attribute(stated, "countBy"), out int countBy) || countBy <= 0)
        {
            return null;
        }

        WordProperty size = _styles.ResolveInDocumentDefaults(runProperty: true, "sz");
        WordProperty fonts = _styles.ResolveInDocumentDefaults(runProperty: true, "rFonts");

        Length emSize = size.IntegerValue is { } halfPoints && halfPoints > 0
            ? Length.FromPoints(halfPoints / 2.0)
            : LineNumbering.DefaultEmSize;

        WordTextStyle style = new(
            Word.Attribute(fonts.Element, "ascii"), emSize, Weight: 400, IsItalic: false, Language: null,
            DeclaredClass: WordParagraphFormats.StatedClass(
                fonts.Element is { } element ? [element] : [], _fontTable));

        if (Face(style) is not { } face) return null;

        return new LineNumbering
        {
            Face = face,
            Font = _references.GetValueOrDefault(style.FaceKey),
            EmSize = emSize,
            CountBy = countBy,
            Start = Word.Integer(Word.Attribute(stated, "start"), out int start) && start > 0
                ? start
                : 1,
            Distance = Word.Integer(Word.Attribute(stated, "distance"), out int distance)
                       && distance > 0
                ? Length.FromTwips(distance)
                : LineNumbering.DefaultDistance,

            // `newSection` maps to false beside `continuous`, because Writer has no per-section restart
            // to map it onto — see the importer's own line, cited above.
            RestartsEachPage =
                string.Equals(Word.Attribute(stated, "restart"), "newPage", StringComparison.Ordinal),
        };
    }

    /// <summary>Reads the body's blocks — its paragraphs and its tables — in document order.</summary>
    /// <param name="body">The <c>w:body</c> element.</param>
    public List<PageBlock> Read(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        _sectionIndex = 0;
        _blocksInSection = 0;
        _pendingBelowTarget = -1;
        _indexFieldDepth = 0;
        _sectionLeftMargins = LeftMargins(body);

        // Before the walk, because a REF may name a bookmark the walk has not reached yet -- which in
        // a document whose references point forward is most of them. See `DocxReferenceFields`.
        _referenceText = DocxReferenceFields.Expansions(body);

        // And before it for the same reason: a paragraph loses its direct formatting because of a
        // TOC field that may be written after it. See `DocxTocStyles`.
        _tocVoidedStyles = DocxTocStyles.Voided(body, _styles);

        // The body is where the document's lists start counting. Reset rather than assumed clean,
        // because the numbering may be the same instance the extraction pass already walked.
        _numbering.ResetCounters();

        List<PageBlock> blocks = [];
        Walk(body, blocks, depth: 0);
        ParagraphBorderJoin.Apply(blocks);
        return blocks;
    }

    /// <summary>
    /// Reads a table cell's blocks, tables included.
    /// </summary>
    /// <remarks>
    /// Unlike <see cref="ReadFlow"/>, which is for a header or a footer, a cell keeps its tables: a table
    /// inside a cell is how a nested table is written, and <see cref="FlowLayouter"/> lays one out. The two
    /// differ only in the list they fill, which is what the generic walk is for.
    /// </remarks>
    /// <param name="element">The cell element.</param>
    public List<PageBlock> ReadCell(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);

        List<PageBlock> blocks = [];
        Walk(element, blocks, depth: 0);
        DropNestedTableFiller(blocks);
        SuppressAutoSpacingInCell(blocks);
        ParagraphBorderJoin.Apply(blocks);
        return blocks;
    }

    /// <summary>
    /// Drops the empty paragraph OOXML makes mandatory after a nested table.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>w:tc</c> may not end with a <c>w:tbl</c>, so Word writes an empty paragraph after every
    /// nested table whether or not the author put one there. LibreOffice does not lay it out, and the
    /// difference is a whole line per nested table — on <c>UG.CAO.00133</c>'s header the row is
    /// 26.35 pt in the reference against our 36.65.
    /// </para>
    /// <para>
    /// <strong>The rule is stated from the reference rather than from the specification</strong>, by
    /// varying what one real cell holds and reading the drawn cell edges back out of both PDFs
    /// (<c>dotnet/probes/words-r44/header-row-mutations.py</c>). Seven variants, one rule fitting all
    /// of them to a tenth of a point:
    /// </para>
    /// <list type="bullet">
    /// <item>table then one empty paragraph — the paragraph is not laid out;</item>
    /// <item>table then a paragraph with text — laid out, so it is emptiness that decides;</item>
    /// <item>table then <em>two</em> empty paragraphs — <strong>both</strong> laid out, because the last
    /// one follows a paragraph rather than a table;</item>
    /// <item>empty paragraph, table, empty paragraph — the leading one is laid out and the trailing one
    /// is not, so it is neither "the cell's first" nor "every empty paragraph".</item>
    /// </list>
    /// <para>
    /// Hence the two conditions together: the <em>last</em> block, and the block before it a table.
    /// Applied in the DOCX reader rather than in the layout because the shape is OOXML's own — an ODF
    /// cell may end with a table, so an empty paragraph after one there is the author's.
    /// </para>
    /// </remarks>
    private static void DropNestedTableFiller(List<PageBlock> blocks)
    {
        if (blocks.Count < 2) return;
        if (blocks[^1] is not PageParagraph last || blocks[^2] is not PageTable) return;
        if (last.Text.Length > 0 || last.Frames.Count > 0 || last.Notes.Count > 0) return;

        blocks.RemoveAt(blocks.Count - 1);
    }

    /// <summary>
    /// Drops the HTML auto margin at a cell's top and bottom edges.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:beforeAutospacing</c> means fourteen points in a body paragraph and <em>nothing</em> on the
    /// first paragraph of a table cell; <c>w:afterAutospacing</c> likewise on the last. LibreOffice
    /// applies both — the first in <c>DomainMapper_Impl::finishParagraph</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:2458-2470</c>, where
    /// <c>bFirstParagraphInCell</c> at matching table depth forces the margin to zero) and the second in
    /// <c>ClearPreviousParagraph</c> (<c>:5457-5468</c>, called from <c>TableManager::closeCell</c>).
    /// Without it every row of a table whose style carries the flag is fourteen points taller than the
    /// document asks for, which on a form of thirty single-line rows is seven pages of invented height.
    /// </para>
    /// <para>
    /// A <em>stated</em> <c>w:before</c> survives, which is why this asks how the margin was arrived at
    /// rather than merely whether it is fourteen points: the suppression is of the auto rule, not of
    /// paragraph spacing in cells.
    /// </para>
    /// <para>
    /// The bottom rule spares a numbered paragraph, exactly as <c>ClearPreviousParagraph</c> does — it
    /// reads the paragraph's numbering rules and leaves the margin alone when it has any.
    /// </para>
    /// <para>
    /// <b>Not done:</b> the same <c>if</c> in <c>finishParagraph</c> also zeroes the top margin of the
    /// first paragraph of a <em>shape</em> and of the first paragraph of the document's first section.
    /// Both are the same rule and both are unimplemented here, because neither was measured — a cell is
    /// where the corpus showed it, and the other two move the body flow, which is not free to change on
    /// an argument from symmetry alone.
    /// </para>
    /// </remarks>
    private void SuppressAutoSpacingInCell(List<PageBlock> blocks)
    {
        // Only a paragraph at the very edge is affected; a nested table there shields whatever follows,
        // because the rule is about the cell's own first and last paragraph.
        if (blocks.Count > 0 && blocks[0] is PageParagraph first
            && WordParagraphFormats.IsAutoSpaced(
                _styles, Word.Child(first.Source as XElement, "pPr"), _tableStyle, before: true))
        {
            blocks[0] = first with { Format = first.Format with { SpaceBefore = Length.Zero } };
        }

        if (blocks.Count > 0 && blocks[^1] is PageParagraph last
            && last.Label is null
            && WordParagraphFormats.IsAutoSpaced(
                _styles, Word.Child(last.Source as XElement, "pPr"), _tableStyle, before: false))
        {
            blocks[^1] = last with { Format = last.Format with { SpaceAfter = Length.Zero } };
        }
    }

    /// <summary>
    /// Reads a flow's blocks: a header's or a footer's.
    /// </summary>
    /// <remarks>
    /// The same walk a cell takes, tables included, because a table is how a two-part running head is usually
    /// laid out — one cell hard left, another hard right — and <see cref="FlowLayouter"/> places one either
    /// way. Dropping the table instead is not the harmless simplification it looks like: its paragraphs would
    /// stack as loose lines, giving the header a height no table has and pushing the body text down by the
    /// difference on every page.
    /// </remarks>
    /// <param name="element">The element whose block-level children to read.</param>
    public List<PageBlock> ReadFlow(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);

        // Each flow numbers its own lists: a numbered paragraph in a footer does not continue the
        // body's count, which is the same rule the extraction reader applies between flows.
        _numbering.ResetCounters();

        // Saved rather than merely cleared, because a text frame's content comes through here too and
        // a frame can sit inside a body paragraph — so an unbalanced field in the frame must not leave
        // the body's own depth raised, and the body's must not reach into the frame.
        int outerIndexFields = _indexFieldDepth;
        _indexFieldDepth = 0;

        try
        {
            List<PageBlock> blocks = [];
            Walk(element, blocks, depth: 0);
            ParagraphBorderJoin.Apply(blocks);
            return blocks;
        }
        finally
        {
            _indexFieldDepth = outerIndexFields;
        }
    }

    /// <summary>
    /// Which section the walk is in, advanced by each paragraph that closes one.
    /// </summary>
    /// <remarks>
    /// A field rather than a walk parameter because the walk recurses through content controls and tracked
    /// insertions, and a section can end inside one — so the count has to survive returning from a nested
    /// call rather than being restored with it.
    /// </remarks>
    private int _sectionIndex;

    /// <summary>
    /// Each section's left margin, in the order the body states them — for a table anchored to the page.
    /// </summary>
    /// <remarks>
    /// The one place this reader needs page geometry. <c>w:horzAnchor="page"</c> measures
    /// <c>w:tblpX</c> from the sheet's own left edge, and everything downstream of here works in the
    /// text area's coordinates, so the margin has to come off before the offset leaves this class. See
    /// <see cref="PositionedLeftEdge"/>.
    /// </remarks>
    private List<Length> _sectionLeftMargins = [];

    /// <summary>The left margin of each <c>w:sectPr</c> in the body, in order.</summary>
    /// <remarks>
    /// Read here rather than taken from the geometry the paginator builds, because a table is read long
    /// before a page exists. A section stating no <c>w:pgMar</c> gets Letter's inch, which is what
    /// <c>SectionPropertyMap</c>'s constructor gives it — see <see cref="PageGeometry.Letter"/>.
    /// </remarks>
    private static List<Length> LeftMargins(XElement body)
        => [.. body
            .Descendants(Word.Name("sectPr"))
            .Select(section => Twips(Word.Child(section, "pgMar"), "left")
                               ?? PageGeometry.Letter.Margins.Left)];

    /// <summary>
    /// The left margin of the section the walk is in, for a page-anchored table's offset.
    /// </summary>
    /// <remarks>
    /// Clamped rather than indexed, because a body's last blocks belong to the section its final
    /// <c>w:sectPr</c> states and <see cref="_sectionIndex"/> has counted past the marks by then.
    /// </remarks>
    private Length SectionLeftMargin
        => _sectionLeftMargins.Count == 0
            ? PageGeometry.Letter.Margins.Left
            : _sectionLeftMargins[Math.Clamp(_sectionIndex, 0, _sectionLeftMargins.Count - 1)];

    /// <summary>
    /// How many blocks the current section has already contributed, reset when a section closes.
    /// </summary>
    /// <remarks>
    /// Only <see cref="IsSectionMarkOnly"/> reads it, and only to answer "is this section mark the whole
    /// section". Counted for every paragraph and table the walk passes rather than for the ones it keeps,
    /// which is what Writer's <c>bIsFirstParaInSection</c> counts — a paragraph the reader could not
    /// resolve a face for still separates the section mark from the start of its section.
    /// </remarks>
    private int _blocksInSection;

    /// <summary>
    /// Whether a paragraph that closes a section is nothing but the section mark, and so is not laid out.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word stores a section break as a paragraph mark carrying <c>w:sectPr</c>, and that mark is not a
    /// paragraph: it takes no line and no spacing. Writer's DOCX importer says so directly — "if the
    /// paragraph contains only the section properties and it has no runs, we should not create a paragraph
    /// for it in Writer, unless that would remove the whole section"
    /// (<c>writerfilter/dmapper/DomainMapper.cxx</c>:4840, the <c>bRemove</c> expression) — and it is the
    /// <c>!bSingleParagraphAfterRedline</c> term there that spells the exception: a mark that is both the
    /// first and the last paragraph of its section is kept, because dropping it would leave the section
    /// with no content to hang a page on.
    /// </para>
    /// <para>
    /// Measured on <c>easa-form-1.docx</c>, whose first section ends with an ordinary empty paragraph and
    /// then a section mark: laying the mark out overflowed the page by one line and produced a sixth page
    /// carrying nothing but the section's footer. LibreOffice's own flat-ODF export of that document holds
    /// one empty paragraph where the DOCX has two, which is this rule visible in its output.
    /// </para>
    /// <para>
    /// "Nothing but the mark" is read from what the paragraph produced rather than from its markup, so a
    /// mark that anchors a frame or cites a note keeps its paragraph: each of those is content the page
    /// would otherwise lose, and Writer guards them individually (<c>HasTopAnchoredObjects</c>,
    /// <c>IsParaWithInlineObject</c>).
    /// </para>
    /// <para>
    /// <strong>A page break does not save the mark, and used to.</strong> The guard list in
    /// <c>bRemove</c> protects a <em>column</em> break — <c>bIsColumnBreak</c>, built from
    /// <c>BreakType_COLUMN_BEFORE</c>/<c>_AFTER</c>/<c>_BOTH</c> a dozen lines above it — and names no
    /// page break at all, so a mark carrying one is removed and the break dies with the paragraph it was
    /// attached to. Measured on the installed 26.2.4.2 by four authored routes onto the same flag, each a
    /// three-paragraph section ending in an empty mark followed by a landscape section: a preceding
    /// <c>&lt;w:br w:type="page"/&gt;</c>, two of them, a <c>w:pageBreakBefore</c> on the mark itself, and
    /// the same on a section already filling its page. LibreOffice emits one portrait page for the first
    /// three and two for the fourth; taking the break at its word emits one more in every case.
    /// </para>
    /// <para>
    /// The corpus case is <c>1_tpr_template__from_fy14_.docx</c>, whose first section ends with a
    /// page-break paragraph and then an empty mark. Its first two pages are word for word and line for
    /// line the reference's; page three held one word, the footer's page number, and pushed the landscape
    /// section that follows onto page four. Nine of the corpus's 200 documents change with this rule.
    /// </para>
    /// <para>
    /// Column breaks are not guarded here because the reader does not model one: only
    /// <c>w:br w:type="page"</c> reaches <see cref="ParagraphFormat.StartsNewPage"/>, so the flag this
    /// test used to read can never have meant a column break. That is a real gap in the other direction —
    /// 26.2.4.2 keeps a mark carrying a column break and gives it a page, and we drop it — and it is
    /// recorded in <c>dotnet/probes/words-e-01/results.md</c> rather than guessed at here.
    /// </para>
    /// </remarks>
    private static bool IsSectionMarkOnly(PageParagraph paragraph)
        => paragraph.Text.Length == 0
           && paragraph.Frames.Count == 0
           && paragraph.Notes.Count == 0;

    /// <summary>
    /// The break each section begins with, indexed as <see cref="_sectionIndex"/> counts them.
    /// </summary>
    /// <remarks>
    /// Only <see cref="HandOnBelowSpacing{T}"/> reads it, and only to tell a continuous break from a
    /// page-starting one. Empty when the caller states nothing, which reads every section as
    /// page-starting — the case the rule was written for, and the one a hand-built source is testing.
    /// </remarks>
    public IReadOnlyList<SectionBreak> SectionBreaks { get; init; } = [];

    /// <summary>
    /// The space-after of a dropped section mark, waiting for the section after it to claim it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A section mark is not laid out — see <see cref="IsSectionMarkOnly"/> — but the space-after it
    /// declares is still the space that ends its section, and Word consolidates it against the space-before
    /// of whatever opens the next one. Writer reproduces that by moving the value onto the last real
    /// paragraph of the closing section before discarding the mark:
    /// <c>DomainMapper_Impl::handleSectPrBeforeRemoval</c> stashes it
    /// (<c>writerfilter/dmapper/DomainMapper_Impl.cxx</c>:10487) and
    /// <c>SectionPropertyMap::EmulateSectPrBelowSpacing</c> writes it to the paragraph before the break
    /// (<c>PropertyMap.cxx</c>:1576), whose own comment is "MS Word is excessively consistent about
    /// consolidating paragraph top and bottom spacing. They even consolidate spacing between section
    /// breaks!"
    /// </para>
    /// <para>
    /// It <em>replaces</em> the previous paragraph's own space-after rather than adding to it, which
    /// Writer justifies with "below spacing before a page break normally has no relevance". Both of
    /// Writer's exclusions are reproduced: a table on either side of the break takes no hand-on, because
    /// a table has neither of the two spacings to consolidate; and a continuous break hands nothing on
    /// unless the paragraph opening the section carries a page break of its own, because without one the
    /// mark's space-after never reaches the layout at all.
    /// </para>
    /// <para>
    /// Measured on <c>03_Technical_Report_(progress)_template.docx</c>, whose landscape section opens with
    /// an 18 pt space-before heading after a mark declaring 6 pt: the reference puts that heading's
    /// baseline 12 pt below where a probe with the space-before removed puts it, which is the consolidated
    /// 18 − 6 and not the 18 the same document's earlier section — whose mark declares no space-after at
    /// all — moves by. Without this the two sections both moved by 18 and the document paginated to
    /// eleven pages against the reference's ten.
    /// </para>
    /// </remarks>
    private Length _pendingBelowSpacing;

    /// <summary>Which block the pending space-after belongs on, or −1 when nothing is pending.</summary>
    private int _pendingBelowTarget = -1;

    /// <summary>
    /// Gives a dropped section mark's space-after to the last paragraph of the section it closed.
    /// </summary>
    /// <param name="into">The blocks read so far.</param>
    /// <param name="startsNewPage">
    /// Whether the paragraph claiming it carries a page break, which is what lets a continuous break hand
    /// anything on.
    /// </param>
    private void HandOnBelowSpacing<T>(List<T> into, bool startsNewPage)
        where T : PageBlock
    {
        int target = _pendingBelowTarget;
        _pendingBelowTarget = -1;

        if (target < 0 || target >= into.Count) return;

        // `_sectionIndex` has already advanced past the mark, so it names the section being opened.
        if (!startsNewPage
            && _sectionIndex < SectionBreaks.Count
            && SectionBreaks[_sectionIndex] == SectionBreak.Continuous)
        {
            return;
        }

        // A table before the break has no space-after to replace, which is Writer's other `FindTableNode`.
        if (into[target] is not PageParagraph last) return;

        if (last with { Format = last.DeclaredFormat with { SpaceAfter = _pendingBelowSpacing } } is T
            replaced)
        {
            into[target] = replaced;
        }
    }

    /// <summary>
    /// How many tables enclose the one being read, counted while its rows are walked.
    /// </summary>
    /// <remarks>
    /// A field for the same reason <see cref="_sectionIndex"/> is one: a cell's blocks are read by the walk
    /// that reads a paragraph's, so a nested table is found several calls deep rather than through a
    /// parameter somebody could pass wrongly. Only the table's own left edge depends on it.
    /// </remarks>
    private int _tableDepth;

    /// <summary>
    /// The <c>w:pPr</c> chain of the table style enclosing the paragraph being read, or null in the body.
    /// </summary>
    /// <remarks>
    /// A field rather than a parameter because a cell's content is read by the same recursive walk the
    /// body uses, and threading it through every overload would touch a dozen signatures for one value
    /// that changes only when a table is entered. Saved and restored around each table, so a nested table
    /// takes its own style and the outer one resumes after it.
    /// </remarks>
    private IReadOnlyList<XElement>? _tableStyle;

    /// <summary>
    /// The table style's <c>w:rPr</c> layers for the cell being read, most specific first, or null in
    /// the body.
    /// </summary>
    /// <remarks>
    /// The companion of <see cref="_tableStyle"/> and a field for the same reason, but it changes per
    /// <em>cell</em> rather than per table: conditional formatting is a property of where the cell sits.
    /// See <see cref="WordTableStyleConditions"/>.
    /// </remarks>
    private IReadOnlyList<XElement>? _tableStyleRun;

    /// <summary>
    /// Walks the body's block-level children.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:sdt</c> — a structured-document tag, which is what a content control is — wraps ordinary
    /// content inside a <c>w:sdtContent</c>, so a walk that stopped at it would lose every paragraph in
    /// a form.
    /// </para>
    /// <para>
    /// Generic in what it fills, which is how one walk serves both the body and a flow. A body takes
    /// <see cref="PageBlock"/> and so keeps the tables; a header, a footer or a cell takes
    /// <see cref="PageParagraph"/>, and a table simply does not fit in the list — so it is dropped by the
    /// type rather than by a flag that could be passed the wrong way round.
    /// </para>
    /// </remarks>
    private void Walk<T>(XElement element, List<T> into, int depth)
        where T : PageBlock
    {
        if (depth > 64 || into.Count >= MaxParagraphs) return;

        foreach (XElement child in element.Elements())
        {
            if (into.Count >= MaxParagraphs) return;

            if (Word.Is(child, "p"))
            {
                bool endsSection = Word.Child(Word.Child(child, "pPr"), "sectPr") is not null;

                // Read it either way: even a paragraph that is only a section mark can leave a page break
                // behind for the paragraph after it, and that bookkeeping lives in `Paragraph`.
                if (Paragraph(child) is { } paragraph)
                {
                    if (endsSection && _blocksInSection > 0 && IsSectionMarkOnly(paragraph))
                    {
                        // Dropped — but its space-after is not dropped with it. See BelowSpacing.
                        _pendingBelowSpacing = paragraph.DeclaredFormat.SpaceAfter;
                        _pendingBelowTarget = into.Count - 1;
                    }
                    else if (paragraph is T block)
                    {
                        HandOnBelowSpacing(into, paragraph.Format.StartsNewPage);
                        into.Add(block);
                    }
                }

                _blocksInSection++;

                // A DOCX states a section's properties at its *end*: the w:sectPr inside a paragraph's
                // properties closes the section that paragraph finishes. So the counter advances after the
                // paragraph, which is what puts that paragraph in the section it ends rather than the next.
                if (endsSection)
                {
                    _sectionIndex++;
                    _blocksInSection = 0;
                }

                continue;
            }

            if (Word.Is(child, "tbl"))
            {
                // A section that starts with a table takes no hand-on: a table has no space-above for the
                // mark's space-after to consolidate against, which is the `FindTableNode` arm of
                // `SectionPropertyMap::EmulateSectPrBelowSpacing`.
                _pendingBelowTarget = -1;

                if (Table(child) is { } table && table is T grid) into.Add(grid);
                _blocksInSection++;
                continue;
            }

            if (Word.Is(child, "sdt") || Word.Is(child, "sdtContent"))
            {
                Walk(child, into, depth + 1);
            }
        }
    }

    /// <summary>
    /// Reads one paragraph, with an optional prefix its own text does not contain.
    /// </summary>
    /// <param name="element">The <c>w:p</c>.</param>
    /// <param name="citation">
    /// The number a <c>w:footnoteRef</c> in this paragraph stands for, or null when it is not a note's. The
    /// number is not in the file: Word marks the place and counts the notes itself.
    /// </param>
    private PageParagraph? Paragraph(XElement element, string? citation = null)
    {
        XElement? properties = DocxTocStyles.Prune(Word.Child(element, "pPr"), _tocVoidedStyles);

        // Two character styles, because `w:pPr/w:rPr` is the *paragraph mark's* formatting and not
        // the paragraph's. ECMA-376 names it "Run Properties for the Paragraph Mark", Word applies
        // it to the pilcrow, and LibreOffice agrees: its flat-ODF export puts it on
        // `loext:marker-style-name` and leaves the text in the paragraph style. Measured on an
        // authored probe against LibreOffice 24.2.7.2 — a bold style whose mark says
        // `<w:b w:val="0"/>` still draws its text in Liberation Sans Bold, and an unstyled
        // paragraph whose mark says `<w:b/><w:sz w:val="48"/>` still draws 10 pt upright.
        //
        // `mark` is not dead weight: an empty paragraph has nothing *but* its mark, and its height
        // is the mark's. Same probe: the mark alone carrying `w:sz w:val="72"` gives the empty
        // paragraph 36 pt of height in the reference.
        //
        // [24.2.7-audit: VERIFIED 2026-08-21, round words-r63 — all three claims re-measured on
        // 26.2.4.2 by `probes/words-r63/audit_markstyle.py`, on the reference alone, each arm with
        // a control whose answer was known first. A `Heavy` paragraph whose mark says
        // `<w:b w:val="0"/>` is drawn in LiberationSans-**Bold** while the same style with the
        // *run* saying it is drawn regular — so the pair separates "the mark formats the text"
        // from "it does not" rather than only confirming one side. A mark saying
        // `<w:b/><w:sz w:val="48"/>` leaves the text LiberationSans at 10.00 pt where the run
        // saying it gives Bold at 24.00 pt. And the empty paragraph's height is the mark's to
        // 0.00 pt: the baseline pitch across it is 52.90 pt against a control's 23.00, a
        // difference of 29.90, so the paragraph itself is 11.50 + 29.90 = 41.40 pt, which is
        // 36 pt x 1.15 exactly. **This was the last open site in this project that a corpus
        // document can reach**: of the other ten hits in it, five sit inside an existing marker
        // and five are prose recording that a superseded-binary measurement has already been
        // re-measured on 26.2.4.2 in the same comment, two of those in readers
        // (`OdtLayoutSource`, `RtfDocumentReader`) whose format has no witness in this corpus at
        // all. See `TODO.24-2-7-audit.md`, round 63.]
        WordTextStyle mark =
            WordParagraphFormats.ResolveText(_styles, properties, _theme, _tableStyleRun, _fontTable);
        WordTextStyle body =
            WordParagraphFormats.ResolveRun(_styles, properties, null, _theme, _tableStyleRun, _fontTable);

        // Both are resolved, not only the one this paragraph draws its text in, because `Face` is
        // also what fills `_references` — and a `FontReference` is the only thing a PDF can turn
        // back into an *embedded* font program. Resolving just the body's face left the list label,
        // which takes the mark's style, with no reference to be embedded through: nine documents
        // went from `match` to `unembedded` on the corpus sweep with the layout otherwise identical.
        OpenTypeFace? face = Face(body);
        OpenTypeFace? markFace = Face(mark);
        if (face is null) return null;

        // Taken before the walk and put back after it, because the walk can set a *new* one. What is
        // read here was left by the paragraph before this one; what the walk leaves belongs to the
        // paragraph after.
        bool breaksPage = _pageBreakPending;
        _pageBreakPending = false;

        RunWalker walker = new(
            CitationOf, Symbol, _constants, _footnoteNumber, _endnoteNumber, StyleReferenceText,
            _indexFieldDepth, ReferenceText,
            linkTarget: LinksResolvable && citation is null ? LinkTarget : null);
        walker.Walk(element, citation);

        // Carried to the paragraph after this one, exactly as the note counters are: a `TOC` field's
        // result is dozens of paragraphs and the flag that strips its entries' character style has to
        // live as long as the field does. See `RunWalker.IndexFieldDepth`.
        _indexFieldDepth = walker.IndexFieldDepth;

        // Where the note's own number landed, for a renumbering pass that has to find it again. A field
        // rather than an out parameter because this method reads an ordinary paragraph and a note's first
        // paragraph alike, and only the call that supplied a citation can have produced one.
        if (citation is not null) _noteCitationOffset = walker.CitationOffset;

        // Notes are numbered across the document, so the counters advance by however many this paragraph
        // referenced — and the bodies are read after the walk, since reading one recurses into this method
        // and would otherwise renumber from the middle of the paragraph that references it.
        _footnoteNumber += walker.FootnotesSeen;
        _endnoteNumber += walker.EndnotesSeen;

        ParagraphFormat format =
            WordParagraphFormats.Resolve(
                _styles, properties, _defaultTabInterval, _autoSpacing, _tableStyle,
                _compatibilityMode >= 15);

        // After the walk, because reading a note body or a text box re-enters this method and a list
        // counter advanced from inside a nested flow would number the paragraph after it wrongly.
        //
        // The mark's style rather than the body's, because a list label takes the formatting of the
        // paragraph mark — which is what `w:pPr/w:rPr` is for, and the one place it is visible.
        (PageLabel? label, format) = ListFormatting(properties, format, mark, markFace ?? face);

        // The runs first, then the text they map: `Apply` rewrites both together, and the offsets it
        // preserves are the ones the notes and frames below were recorded against.
        List<PageRun> runs = RunsOf(walker.Ranges, properties, body, face, walker.Text);
        string mapped = CaseMapping.Apply(walker.Text, runs);

        // A paragraph with nothing in it is its mark, so that is what sizes it; one with text is
        // sized by the text, and its mark formats a pilcrow nobody draws.
        // A paragraph holding nothing but floating drawings is empty: they put no character in the
        // text — see the `case "drawing"` in the walk below — so this is the same test.
        bool empty = walker.Text.Length == 0;
        WordTextStyle text = empty ? mark : body;
        if (empty) face = markFace ?? face;

        PageParagraph read = new()
        {
            SectionIndex = _sectionIndex,
            Text = mapped,
            Face = face,
            Font = _references.GetValueOrDefault(text.FaceKey),
            Colour = text.Colour ?? Colour.Transparent,
            Shading = ShadeColour(WordParagraphFormats.ShadingOf(_styles, properties)),
            Borders = ParagraphBorders(properties),
            Format = breaksPage || walker.BreaksPageHere
                ? format with { StartsNewPage = true }
                : format,
            Label = label,

            // #i3952#: a tab or a run of spaces does not raise a line's height in a Word document, and a
            // DOCX imports with the setting on. See PageParagraph.BlanksAreTransparentToHeight.
            BlanksAreTransparentToHeight = true,

            // A toggle in the schema, so `<w:suppressLineNumbers/>` and `w:val="1"` both mean on and
            // `w:val="0"` — which a style commonly writes to undo an inherited one — means off.
            SuppressesLineNumbers = _styles
                .ResolveParagraphProperty(
                    "suppressLineNumbers",
                    properties,
                    Word.Value(properties, "pStyle") ?? _styles.DefaultStyleId(WordStyleType.Paragraph))
                .IsOn,
            Metrics = _metrics,
            Fallback = _fonts,

            // Word's "add space between Asian and Western text". See ScriptSpacing; the DOC reader
            // sets it for the same reason and the ODF one does not.
            AddsScriptSpace = true,
            Item = text.FontItem,
            EmSize = text.Size,
            Language = text.Language,
            Shaping = new ShapingOptions(
                Language: text.Language, DisableKerning: !text.AutoKerning),
            Tracking = text.Tracking,

            // The paragraph mark's own character width, for the same reason as its tracking: a
            // paragraph set end to end in one scaled style carries no runs at all.
            WidthPerCent = text.WidthPerCent,
            Runs = runs,
            Fields = walker.Fields,
            Notes = NotesOf(walker.Notes),
            Frames = FramesOf(walker.Frames, walker.CheckBoxes, properties),
            Anchors = walker.Anchors,
            OutlineLevel = OutlineLevelOf(properties),
            Source = element,
        };

        // After the note bodies and the text boxes above, which recurse into this method and share the
        // field. Writer ignores a page break inside either — `DomainMapper.cxx:4376` applies a deferred
        // one only when it is not in a footnote, a shape or a comment — and overwriting here is what
        // makes that true here too: whatever a nested flow left behind is replaced by this paragraph's
        // own answer, so a break inside a caption cannot push the paragraph after the caption's frame.
        _pageBreakPending = walker.BreaksPage;

        // After the walk, so a STYLEREF in *this* paragraph still quotes the one before it. Word's own
        // search runs backwards from the field, and a heading whose own text is what a caption inside it
        // would quote does not arise.
        if (mapped.Length > 0 && Word.Value(properties, "pStyle") is { Length: > 0 } styled)
        {
            _styleText[styled] = mapped;
        }

        return read;
    }

    /// <summary>
    /// The text last read in each paragraph style, which is what a <c>STYLEREF</c> quotes.
    /// </summary>
    /// <remarks>
    /// The most recent one rather than all of them, because the search is backwards from the field —
    /// <c>SwGetRefFieldType::FindAnchorRefStyleOther</c> scans from the field's own node towards the
    /// start and takes the first match. Keeping the last text seen per style answers that in one lookup
    /// and costs one dictionary entry per style the document actually uses.
    /// </remarks>
    private readonly Dictionary<string, string> _styleText = [];

    /// <summary>
    /// What a <c>STYLEREF</c> naming a style quotes, or null when nothing in that style has been read.
    /// </summary>
    /// <remarks>
    /// The name a field states is a style <em>name</em> — or Word's undocumented bare digit for a
    /// built-in heading level — while a paragraph names a style <em>id</em>, so the two are matched
    /// through the style table rather than directly. Null when the style is unknown or nothing precedes
    /// the field in it, which leaves the producer's cached result in place: a wrong substitution is
    /// worse than a stale one.
    /// </remarks>
    /// <param name="name">The style the field named.</param>
    private string? StyleReferenceText(string name)
    {
        // Not in a running head, where this map cannot answer. A header is read once per section, after
        // the body walk has finished (`OoxmlWordDocument.Layout` calls `source.Read(body)` before
        // `Paginated`), so `_styleText` holds the *last* paragraph in each style in the whole document
        // — an answer that is right on at most one page and wrong on all the others. Writer resolves a
        // STYLEREF from the field's own position instead, scanning backwards from it
        // (`SwGetRefFieldType::FindAnchorRefStyleOther`), which for a running head means "the heading
        // in force on this page" and is a different answer on every page.
        //
        // Measured on `231164_SystemDesignDocument.docx`: the reference's running head reads
        // "List of Figures", "Introduction", "General Overview and Design Guidelines/Approach" on
        // pages 3, 4 and 5; this map answers "List of Tables" and "External Interfaces" — the last
        // front-matter heading and the last `Heading 2` — on all of them. Declining leaves the
        // producer's cached result, which is the stated policy of this method ("a wrong substitution is
        // worse than a stale one") and is what Word wrote for the page the field was last laid out on.
        // Closing this properly needs a per-page substitution like `PageFields`; see findings.md.
        if (InHeaderFooter) return null;

        if (_styleText.Count == 0) return null;
        if (_styleText.TryGetValue(name, out string? byId)) return byId;

        // "1" through "9" mean the built-in heading of that level, which is a style whose *name* is
        // "heading N" whatever the document calls its id.
        string wanted = name.Length == 1 && name[0] is >= '1' and <= '9'
            ? "heading " + name
            : name;

        foreach (WordStyle style in _styles.All)
        {
            if (style.Type != WordStyleType.Paragraph) continue;
            if (!string.Equals(style.Name, wanted, StringComparison.OrdinalIgnoreCase)) continue;
            if (_styleText.TryGetValue(style.StyleId, out string? byName)) return byName;
        }

        return null;
    }

    /// <summary>
    /// What each <c>REF</c>-named bookmark expands to, or null when the body quotes none.
    /// </summary>
    /// <remarks>
    /// Document-level and computed up front, unlike <see cref="_styleText"/>: a <c>STYLEREF</c> quotes
    /// backwards and a <c>REF</c> quotes a name, so the one can be answered as the walk goes and the
    /// other cannot. Survives into the headers and footers, which are read after the body and may
    /// quote a bookmark in it.
    /// </remarks>
    private Dictionary<string, string>? _referenceText;

    /// <summary>
    /// The text a <c>REF</c> naming a bookmark draws, or null when this document holds no such mark.
    /// </summary>
    /// <remarks>
    /// Null leaves the producer's cached result in place, which is this reader's standing policy for a
    /// field it cannot compute: our own bookmark table is what the lookup missed, and Writer's
    /// "Error: Reference source not found" would be a confident wrong answer where the cache is a
    /// stale right one.
    /// </remarks>
    /// <param name="name">The bookmark the field named.</param>
    private string? ReferenceText(string name)
        => _referenceText is not null && _referenceText.TryGetValue(name, out string? text) ? text : null;

    /// <summary>
    /// The styles whose paragraphs draw none of their own direct paragraph formatting.
    /// </summary>
    /// <remarks>
    /// Empty for every document that states no <c>TOC … \t</c> switch, which is 250 of the corpus's
    /// 271 DOCX. See <see cref="DocxTocStyles"/> for the measurement and for why this is here.
    /// </remarks>
    private IReadOnlySet<string> _tocVoidedStyles = new HashSet<string>(StringComparer.Ordinal);

    /// <summary>Whether the paragraph read next begins a page, because the one before ended with a break.</summary>
    private bool _pageBreakPending;

    /// <summary>How many footnotes the walk has passed, counted across the document.</summary>
    private int _footnoteNumber;

    /// <summary>
    /// How many <c>TOC</c>, <c>INDEX</c> or <c>BIBLIOGRAPHY</c> results the walk is inside.
    /// </summary>
    /// <remarks>
    /// Document-level rather than per paragraph, because the field is: its <c>fldChar begin</c> shares a
    /// paragraph with the first contents entry and its <c>end</c> with the last, and everything between
    /// has to know it is inside an index result or it takes the <c>Hyperlink</c> character style Word
    /// wrote on it. LibreOffice keeps the same flag on the import state — see
    /// <c>RunWalker._indexResults</c> for the citations and for what it costs to get wrong.
    /// </remarks>
    private int _indexFieldDepth;

    /// <summary>
    /// Where the last note body's own citation was emitted, or −1 when it emitted none.
    /// </summary>
    /// <remarks>
    /// A DOCX marks the place: a <c>w:footnoteRef</c> in the note's first paragraph, which a note beginning
    /// with a tab puts at one rather than at nought. Recorded so that a renumbering pass can rewrite the
    /// number at the head of the note as well as the one in the sentence.
    /// </remarks>
    private int _noteCitationOffset = -1;

    /// <summary>
    /// The number the next endnote is cited by, counted separately from the footnotes.
    /// </summary>
    /// <remarks>
    /// Its own counter because the two sequences are independent — a document with two footnotes and two
    /// endnotes cites 1, 2, i and ii, not 1, 2, iii and iv — and because they are formatted differently.
    /// </remarks>
    private int _endnoteNumber;

    /// <summary>
    /// Reads each referenced note's body from the document's notes part.
    /// </summary>
    /// <remarks>
    /// By <c>w:id</c>, which is what a DOCX gives instead of putting the body at the reference: the note
    /// lives in <c>footnotes.xml</c> and the sentence holds only its number. The citation is placed at the
    /// head of the note's first paragraph, which is where Word draws it and where the part does not have it.
    /// </remarks>
    private List<PageNote> NotesOf(List<NoteAnchor> anchors)
    {
        if (anchors.Count == 0) return [];

        List<PageNote> notes = new(anchors.Count);

        foreach (NoteAnchor anchor in anchors)
        {
            if (anchor.Id is null) continue;

            IReadOnlyDictionary<string, XElement> part =
                anchor.IsEndnote ? _endnotes : _footnotes;

            if (!part.TryGetValue(anchor.Id, out XElement? body)) continue;

            List<PageBlock> blocks = ReadNoteBody(body, anchor.Citation, out int bodyOffset);
            if (blocks.Count == 0) continue;

            notes.Add(new PageNote
            {
                Blocks = blocks,
                Offset = anchor.Offset,
                IsEndnote = anchor.IsEndnote,
                Placement =
                    (anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering).Placement,
                Restart = (anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering).Restart,
                Numbering = anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering,
                Citation = anchor.Citation,
                BodyOffset = bodyOffset,
            });
        }

        return notes;
    }

    /// <summary>
    /// Reads a note's body, putting the citation at the head of its first paragraph.
    /// </summary>
    /// <remarks>
    /// Its own walk rather than <see cref="ReadCell"/>'s, only because the first paragraph takes the
    /// citation and the rest do not — everything else about it is the same, tables included.
    /// </remarks>
    private List<PageBlock> ReadNoteBody(XElement body, string citation, out int citationOffset)
    {
        List<PageBlock> blocks = [];
        bool first = true;

        _noteCitationOffset = -1;
        _noteDepth++;

        try
        {
            return ReadNoteBlocks(body, citation, blocks, first, out citationOffset);
        }
        finally
        {
            _noteDepth--;
        }
    }

    private List<PageBlock> ReadNoteBlocks(
        XElement body, string citation, List<PageBlock> blocks, bool first, out int citationOffset)
    {
        foreach (XElement child in body.Elements())
        {
            OfficeOperation.Poll();
            if (Word.Is(child, "p"))
            {
                PageParagraph? paragraph = first ? Paragraph(child, citation) : Paragraph(child);

                if (paragraph is not null)
                {
                    blocks.Add(paragraph);
                    first = false;
                }

                continue;
            }

            Walk(child, blocks, depth: 0);
        }

        // Nought when the part marks no place for its number, which is what a note whose first paragraph
        // holds no `w:footnoteRef` is: the citation was never emitted and there is nothing to rewrite.
        citationOffset = Math.Max(0, _noteCitationOffset);
        ParagraphBorderJoin.Apply(blocks);
        return blocks;
    }

    /// <summary>
    /// The paragraph's runs, or nothing when every one of them is the paragraph's own formatting.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Returning an empty list for a uniform paragraph is not only an optimisation: it puts plain prose
    /// back on the single-face path, which shapes the whole paragraph in one call. A run boundary also
    /// breaks shaping context, so a paragraph split into runs it does not need loses a kern pair at each
    /// boundary and measures very slightly wide — and a DOCX splits runs for reasons that have nothing to
    /// do with formatting, a spell-check marker or a revision id being enough.
    /// </para>
    /// <para>
    /// A range whose font cannot be loaded falls back to the paragraph's face rather than being dropped:
    /// its text is still part of the paragraph, and losing it would silently shorten the document.
    /// </para>
    /// </remarks>
    /// <param name="ranges">The stretches of the paragraph and the run properties over each.</param>
    /// <param name="paragraphProperties">The paragraph's <c>w:pPr</c>.</param>
    /// <param name="paragraph">The paragraph mark's own style, which an unstyled run inherits.</param>
    /// <param name="paragraphFace">The face the paragraph is set in, for a run whose own cannot load.</param>
    /// <param name="text">
    /// The paragraph's text, which the ranges index into. It is needed because a run's script
    /// decides <em>which of Writer's three character-font items</em> the run is set from, and the
    /// item carries its own family class and its own language — see
    /// <see cref="WriterScripts.ForRun"/>.
    /// </param>
    private List<PageRun> RunsOf(
        IReadOnlyList<StyledRange> ranges,
        XElement? paragraphProperties,
        WordTextStyle paragraph,
        OpenTypeFace paragraphFace,
        string text)
    {
        List<PageRun> runs = new(ranges.Count);
        bool varies = false;
        FontReference? paragraphFont = _references.GetValueOrDefault(paragraph.FaceKey);

        foreach (StyledRange range in ranges)
        {
            WordTextStyle style = range.RunProperties is null
                ? paragraph
                : WordParagraphFormats.ResolveRun(
                    _styles, paragraphProperties, range.RunProperties, _theme, _tableStyleRun,
                    _fontTable, ignoreCharacterStyle: range.InIndexField);

            if (range.IsCitation) style = AsCitation(style);

            // Which font item this run's text selects. A run of Latin prose selects the western one
            // and nothing changes; a run of the ambiguous characters Word marks `w:hint="eastAsia"`
            // selects the East Asian item, which carries neither the western item's roman default
            // nor its language.
            int from = Math.Clamp(range.Start, 0, text.Length);
            style = style.OnScript(text.AsSpan(from, Math.Clamp(range.Length, 0, text.Length - from)));

            // A `w:sym` names its own face for one character, and it was resolved when the character was
            // chosen. Everything else about the run — its size, its colour, its escapement — still comes
            // from the run, so only the face is taken from the symbol.
            OpenTypeFace face = range.Symbol?.Face ?? Face(style) ?? paragraphFace;

            // Resolved before the predicate rather than inside the constructor call below, because the
            // predicate needs it: `Face` is what fills `_references`, so the lookup is only valid here.
            FontReference? font = range.Symbol is { } named
                ? named.Font
                : _references.GetValueOrDefault(style.FaceKey);

            // The escapement is resolved here rather than where it was read, because its rise is a fraction
            // of the face's height and the face is only known now.
            Length size = style.Escapement.SizeOf(style.Size);
            Length rise = style.Escapement.RiseOf(face, style.Size);

            if (face != paragraphFace
                // A symbol's face is its own even when it happens to equal the paragraph's: losing the
                // runs here would draw its code point out of whatever the paragraph is set in, which for
                // a Private Use Area slot is .notdef.
                || range.Symbol is not null
                || size != paragraph.Size
                || style.Colour != paragraph.Colour
                || style.Language != paragraph.Language
                // And the font item, which decides where a character this face cannot draw is
                // looked for. Two runs can resolve to the *same* face off different items -- a
                // `w:hint="eastAsia"` run naming the paragraph's own family is the shape -- so
                // nothing above can see the difference, and folding them would leave the run
                // asking under the paragraph mark's western item.
                || style.FontItem != paragraph.FontItem
                || rise != Length.Zero
                // A case map has to survive the uniform-paragraph shortcut: it is the one property here
                // that changes the *characters*, so dropping the runs would draw the text as stored.
                || style.CaseMap != PageCaseMap.None
                // So does a highlight: the paragraph carries none of its own, so a paragraph highlighted
                // end to end is uniform by every other test and would lose its band entirely.
                || style.Highlight is not null
                // And so do the two rules, for the same reason: neither changes a width, so a paragraph
                // underlined end to end is uniform by every measurement test and would be drawn plain.
                || style.Underline != TextUnderline.None
                || style.IsStruckThrough
                // Kerning, unlike the two rules, does change a measurement — so a run that kerns
                // inside a paragraph that does not has to survive the shortcut or its width is the
                // paragraph's answer rather than its own.
                || style.AutoKerning != paragraph.AutoKerning
                // And tracking, for the same reason and more sharply: it is a distance per character,
                // so a run that disagrees with its paragraph mark is wrong by its own length.
                || style.Tracking != paragraph.Tracking
                // And a character width, which is tracking's twin in this respect and worse in one:
                // it multiplies the run's whole advance rather than adding to it, so a 99 per cent run
                // measured at the paragraph's 100 is out by its own length and a quarter.
                || style.WidthPerCent != paragraph.WidthPerCent
                // And a synthetic oblique, which is drawing-only in the same way and was the one
                // missing from this list: an italic run whose family has no italic installed resolves to
                // the *same* face as its upright neighbour, so nothing above can see it and the fold
                // would draw it upright. See PageRun.LeansDifferently.
                || PageRun.LeansDifferently(font, paragraphFont)
                // A link has to keep its own run even when it is formatted exactly like the text around
                // it, because the run is what the clickable area is cut from.
                || range.Link is not null)
            {
                varies = true;
            }

            runs.Add(new PageRun(
                range.Start,
                range.Length,
                face,
                size,
                font,
                style.Colour ?? paragraph.Colour ?? Colour.Transparent,
                new ShapingOptions(Language: style.Language, DisableKerning: !style.AutoKerning),
                rise,
                style.CaseMap,
                Highlight: style.Highlight ?? default,
                Underline: style.Underline,
                IsStruckThrough: style.IsStruckThrough,
                Tracking: style.Tracking,
                WidthPerCent: style.WidthPerCent,
                Item: style.FontItem,
                Link: range.Link));
        }

        return varies ? runs : [];
    }

    /// <summary>
    /// A stretch of a paragraph's text and the run properties in force over it.
    /// </summary>
    /// <param name="Start">Its first character, as an index into the paragraph's text.</param>
    /// <param name="Length">How many characters it covers.</param>
    /// <param name="RunProperties">
    /// The enclosing <c>w:r</c>'s <c>w:rPr</c>, or null when the run states none — in which case the
    /// paragraph mark's own formatting applies.
    /// </param>
    /// <param name="IsCitation">
    /// True for a note's citation, which Word draws superscript whether the run says so or not.
    /// </param>
    /// <param name="Symbol">
    /// The face a <c>w:sym</c> resolved to, or null for ordinary text. Carried already resolved because
    /// the same decision picks the character: a slot recoded into OpenSymbol is a different code point
    /// from the one the file states, so the face cannot be chosen after the text is built.
    /// </param>
    /// <param name="InIndexField">
    /// True when the range sits in the *result* of a <c>TOC</c>, <c>INDEX</c> or <c>BIBLIOGRAPHY</c>
    /// field, where the character style a run names is not applied. See <see cref="RunWalker"/>.
    /// </param>
    /// <param name="Link">
    /// Where the text leads when it is clicked: an address, <c>#name</c> for a bookmark in this document,
    /// or null for text that is not a link. Two ranges that differ only in this do not merge.
    /// </param>
    private readonly record struct StyledRange(
        int Start,
        int Length,
        XElement? RunProperties,
        bool IsCitation = false,
        (OpenTypeFace Face, FontReference? Font)? Symbol = null,
        bool InIndexField = false,
        string? Link = null);

    /// <summary>A note found while walking a paragraph, before its body has been read.</summary>
    /// <param name="Offset">Where its citation sits in the paragraph's text.</param>
    /// <param name="Id">The <c>w:id</c> naming its body in the notes part.</param>
    /// <param name="IsEndnote">True for an endnote, whose body lives in a different part.</param>
    /// <param name="Citation">The number it is cited by, counted rather than read, and already formatted.</param>
    private readonly record struct NoteAnchor(int Offset, string? Id, bool IsEndnote, string Citation);

    /// <summary>One floating frame in a paragraph, with the character offset it is anchored at.</summary>
    private readonly record struct FrameAnchor(int Offset, XElement Element);

    /// <summary>
    /// How deeply a frame's own text may hold further frames before the innermost is dropped.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input: a text frame holds paragraphs, a paragraph holds drawings, and a file
    /// claiming a hundred levels would read the same walk a hundred deep. Real documents nest one.
    /// </remarks>
    private const int MaxFrameNesting = 8;

    /// <summary>How many frames enclose the paragraph currently being read.</summary>
    private int _frameDepth;

    /// <summary>
    /// Whether the walk is inside a header or a footer part.
    /// </summary>
    /// <remarks>
    /// Set by the reader around its <see cref="ReadFlow"/> call and restored afterwards, the same shape
    /// as <c>DocxPictures.Scope</c> beside it and for the same reason: the part is what knows, and the
    /// walk cannot tell — a header's body is an ordinary block sequence. It decides one thing, the paint
    /// order of the drawings it anchors (<see cref="PageFrame.BehindText"/>), so a caller that forgets
    /// to set it gets the body's answer rather than a wrong one.
    /// </remarks>
    public bool InHeaderFooter { get; set; }

    /// <summary>
    /// Resolves a relationship id of the main document part to the external address it names, or is
    /// null when links are not wanted. See <see cref="LinksResolvable"/> for where it is not consulted.
    /// </summary>
    public Func<string, string?>? LinkTarget { get; init; }

    /// <summary>How many notes' bodies enclose the paragraph being read.</summary>
    private int _noteDepth;

    /// <summary>
    /// Whether a relationship id in the paragraph being read is one of the main document's.
    /// </summary>
    /// <remarks>
    /// Ids are scoped to a part: <c>rId5</c> in <c>header1.xml</c> names a different relationship from
    /// <c>rId5</c> in the body. Resolving a header's or a note's id against the body's table would link
    /// the text to a plausible, wrong address, which is worse than not linking it.
    /// </remarks>
    private bool LinksResolvable => !InHeaderFooter && _noteDepth == 0;

    /// <summary>The outline level of a paragraph, 1 to 9, or 0 when it is body text.</summary>
    /// <remarks>
    /// <c>w:outlineLvl</c> is zero-based and lives in the paragraph's properties or, far more often, in
    /// its style. Level 9 (stored as 9) is WordprocessingML's "body text" and is not an entry.
    /// </remarks>
    private int OutlineLevelOf(XElement? properties)
    {
        string? styleId = Word.Value(properties, "pStyle") ?? _styles.DefaultStyleId(WordStyleType.Paragraph);
        return _styles.ResolveParagraphProperty("outlineLvl", properties, styleId).IntegerValue is >= 0 and <= 8 and { } level
            ? level + 1
            : 0;
    }

    /// <summary>
    /// Reads the frames a paragraph anchors, with their own text laid out inside them.
    /// </summary>
    /// <remarks>
    /// A frame's content goes through <see cref="ReadFlow"/> — the same walk a header takes — so a frame
    /// containing a table or a list needs nothing of its own. The reader therefore re-enters itself, which
    /// is why the depth is counted.
    /// </remarks>
    /// <summary>
    /// The square a legacy <c>FORMCHECKBOX</c> puts on the line, or null when its face is unresolvable.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwFieldFormCheckboxPortion::Format</c> (<c>sw/source/core/text/portxt.cxx</c>:1492) sets the
    /// portion's width and height to <c>rInf.GetTextHeight()</c> and its ascent to
    /// <c>rInf.GetAscent()</c> — <em>"the width of the checkbox portion is the same as its height since
    /// it's a square and that size depends on the font size"</em> — and
    /// <c>SwTextPaintInfo::DrawCheckBox</c> (<c>inftxt.cxx</c>:1247) then strokes that rectangle
    /// deflated by 25 twips on each side, in black with no fill, plus both diagonals when it is ticked.
    /// </para>
    /// <para>
    /// <strong>The standing record said this "would not pin (9.0…15.9 pt, not following
    /// <c>w:checkBox/w:size</c>)" and left 675 fields in 12 documents undrawn. It pins exactly, and
    /// <c>w:checkBox/w:size</c> is inert.</strong> Measured on 26.2.4.2,
    /// <c>probes/words-r56/formcheckbox.py</c>, nineteen authored packages with a duplicate-input
    /// control that agreed to the digit: at 12 pt Liberation Serif the square is 11.300 pt against a
    /// 276-twip text height, at 8 pt 6.700 against 184, at 24 pt 25.100 against 552 and at 40 pt 43.500
    /// against 920 — a constant 50 twips less, at every size. Four fixtures stating
    /// <c>w:size</c> of 5, 10, 20 and 40 pt all draw the run's own 11.300, and the square changes with
    /// the <em>face</em> at one size — Liberation Mono 11.100, DejaVu Sans 11.500, Carlito 12.150 —
    /// which is what says it follows the line's text height rather than anything the field declares.
    /// So 9.0…15.9 pt was a range of font sizes read as a failure to pin.
    /// </para>
    /// <para>
    /// The width matters as much as the box: 675 of these across 12 corpus documents were taking no
    /// room at all, so every line holding one was laid out narrower than the reference lays it out.
    /// </para>
    /// </remarks>
    private PageFrame? CheckBoxFrame(CheckBoxAnchor box, XElement? paragraphProperties)
    {
        WordTextStyle style = WordParagraphFormats.ResolveRun(
            _styles, paragraphProperties, box.RunProperties, _theme, _tableStyleRun, _fontTable);

        if (Face(style) is not { } face) return null;

        LineMetrics metrics = LineSpacing.Resolve(face, _metrics, WriterLineBox.LeadingAboveText);
        Length side = metrics.ScaledLineHeight(style.Size);
        if (side <= CheckBoxInset + CheckBoxInset) return null;

        return new PageFrame
        {
            Size = new Core.Geometry.DocSize(side, side),
            Anchor = Layout.FrameAnchor.AsCharacter,
            AnchorOffset = box.Offset,
            InlineAscent = metrics.ScaledAscent(style.Size),
            BorderColour = Colour.Black,
            BorderWidth = CheckBoxStroke,
            BorderInset = CheckBoxInset,
            IsCrossed = box.IsChecked,
            IsTextPortion = true,
        };
    }

    /// <summary>How far inside the portion the box is stroked — <c>DrawCheckBox</c>'s own 25 twips.</summary>
    private static readonly Length CheckBoxInset = Length.FromTwips(25);

    /// <summary>
    /// The hairline the reference strokes it with — the 0.1 pt its PDF writer sets for a nought-width pen.
    /// </summary>
    private static readonly Length CheckBoxStroke = Length.FromPoints(0.1);

    /// <summary>A legacy form checkbox, and where in the paragraph's text it sits.</summary>
    /// <param name="Offset">The anchor character it stands behind.</param>
    /// <param name="RunProperties">The <c>w:rPr</c> of the run holding its <c>w:fldChar</c>.</param>
    /// <param name="IsChecked">Whether it is ticked, so that it is crossed as well as bordered.</param>
    private sealed record CheckBoxAnchor(int Offset, XElement? RunProperties, bool IsChecked);

    /// <summary>
    /// The room a formula takes on the line it sits on — nothing drawn, and no width.
    /// </summary>
    /// <remarks>
    /// <para>
    /// 26.2.4.2 does not lay an OMML formula out as text. Its importer builds a StarMath document
    /// from the subtree and embeds it as an OLE object anchored as-character, so the line takes the
    /// <em>object's</em> height — see <see cref="OfficeMathBox"/>, which is that height and which
    /// arm of it is measured against what.
    /// </para>
    /// <para>
    /// <strong>Zero width, because the marks are drawn as text anyway.</strong> This tree emits the
    /// formula's own glyphs into the paragraph (<c>AppendMath</c>), so the advance is already paid
    /// for; the frame contributes the <em>height</em> and nothing else. A frame that also claimed
    /// the object's width would charge the line for the formula twice.
    /// </para>
    /// <para>
    /// <strong>What it is worth.</strong> Our line reserved 11.90 pt for every shape at the default
    /// size — a nested fraction cost exactly what a bare <c>x</c> did, and the reserved box was
    /// shorter than the ink we drew into it. On <c>ABCD-FE-01-00 Flight Envelope</c>'s page 6 that
    /// is 8.24 pt of slack across three formula paragraphs, which is the whole of the blank page
    /// the reference emits and we did not: the slack sweep puts the reference's threshold at
    /// 8.50 pt and ours at 16.75, either side of the document's own 10.008 pt.
    /// <c>probes/blankpage-r163/results.md</c> §4.
    /// </para>
    /// </remarks>
    private static PageFrame? MathFrame(FrameAnchor anchor)
    {
        if (OfficeMathBox.Measure(anchor.Element) is not { } extent) return null;

        return new PageFrame
        {
            Size = new Core.Geometry.DocSize(Length.Zero, extent.Height),
            Anchor = Layout.FrameAnchor.AsCharacter,
            AnchorOffset = anchor.Offset,
            InlineAscent = extent.Ascent,
            IsTextPortion = true,
        };
    }

    private List<PageFrame> FramesOf(
        List<FrameAnchor> anchors,
        List<CheckBoxAnchor> checkBoxes,
        XElement? paragraphProperties)
    {
        if (anchors.Count == 0 && checkBoxes.Count == 0) return [];

        List<PageFrame> frames = [];

        foreach (CheckBoxAnchor box in checkBoxes)
        {
            if (CheckBoxFrame(box, paragraphProperties) is { } drawn) frames.Add(drawn);
        }

        foreach (FrameAnchor anchor in anchors)
        {
            Func<XElement, IReadOnlyList<PageBlock>>? content =
                _frameDepth < MaxFrameNesting ? Content : null;

            if (anchor.Element.Name.NamespaceName == OoxmlNamespaces.OfficeMath)
            {
                if (MathFrame(anchor) is { } formula) frames.Add(formula);
                continue;
            }

            // VML states its geometry differently from DrawingML and most of it reserves nothing, so
            // it has a reader of its own rather than a branch inside `DocxFrames`.
            if (anchor.Element.Name.LocalName is "pict" or "object")
            {
                frames.AddRange(
                    DocxVmlFrames.ReadAll(anchor.Element, anchor.Offset, Pictures, content, VmlFace));
                continue;
            }

            frames.AddRange(DocxFrames.ReadAll(
                anchor.Element, content, anchor.Offset, Pictures,
                new DocxFrameContext(_theme, InHeaderFooter, _compatibilityMode)
                {
                    Styles = _shapeStyles,
                }));
        }

        return frames;

        IReadOnlyList<PageBlock> Content(XElement box)
        {
            _frameDepth++;
            try
            {
                return ReadFlow(box);
            }
            finally
            {
                _frameDepth--;
            }
        }
    }

    /// <summary>
    /// Walks a paragraph, building the text as laid out and the ranges its runs divide it into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two things have to be got right or the measurement is of the wrong string. A <c>w:del</c> holds
    /// text a tracked change removed, and it is still in the file — measuring it lays out words the
    /// document does not show. And a field's instruction lives in the same run sequence as its result,
    /// bracketed by <c>w:fldChar</c> markers, so a reader that takes every <c>w:t</c> lays out
    /// <c>PAGE \* Arabic</c> in the middle of a sentence.
    /// </para>
    /// <para>
    /// <c>w:tab</c> and <c>w:br</c> are elements rather than characters, as in ODF, and dropping them
    /// silently closes up the space they occupy.
    /// </para>
    /// <para>
    /// The ranges come from the same walk rather than from a second pass, because they are offsets into
    /// that text and the text is not a concatenation of the paragraph's <c>w:t</c> values — every tab,
    /// break and anchor shifts everything after it, and every skipped deletion shifts it back.
    /// </para>
    /// </remarks>
    private sealed class RunWalker
    {
        /// <summary>Creates a walker.</summary>
        /// <param name="citation">How a note of a class and an index is cited.</param>
        /// <param name="footnote">
        /// How many footnotes came before this paragraph. Passed in because notes are numbered across the
        /// document rather than within a paragraph, so the counters belong to the source.
        /// </param>
        /// <param name="symbol">
        /// How a <c>w:sym</c>'s face and slot resolve to something drawable, or null when nothing can
        /// draw it. Supplied by the source for the same reason the citation is: font resolution is the
        /// source's, and the answer decides the *characters* — a recoded slot is a different code point
        /// from the one the file states — so it cannot be deferred to the pass that styles the ranges.
        /// </param>
        /// <param name="endnote">How many endnotes came before it, counted separately.</param>
        /// <param name="constants">What a <c>FILENAME</c> or <c>TITLE</c> field evaluates to.</param>
        /// <param name="styleReference">
        /// What a <c>STYLEREF</c> naming a style quotes, or null when nothing has been read in that
        /// style yet. Supplied by the source because the answer is a paragraph this walker never sees.
        /// </param>
        /// <param name="referenceText">
        /// What a <c>REF</c> naming a bookmark draws, or null when the document holds no such mark.
        /// Supplied by the source because the answer is a bookmark in another paragraph, which a
        /// per-paragraph walker cannot see.
        /// </param>
        /// <param name="linkTarget">
        /// Resolves a relationship id to the external address it names, or null where the paragraph's
        /// own part is not the body and the ids would be read against the wrong table.
        /// </param>
        /// <param name="indexFieldDepth">
        /// How many index fields' results were still open when the paragraph before this one ended.
        /// A walker is built per paragraph and a <c>TOC</c> field's result is dozens of them, so
        /// without this the suppression below would reach only the entry that shares a paragraph with
        /// the field's own <c>fldChar begin</c>. See <see cref="_indexResults"/>.
        /// </param>
        internal RunWalker(
            Func<bool, int, string> citation,
            Func<string?, char, (string Text, OpenTypeFace Face, FontReference? Font)?> symbol,
            ConstantFields constants = default,
            int footnote = 0,
            int endnote = 0,
            Func<string, string?>? styleReference = null,
            int indexFieldDepth = 0,
            Func<string, string?>? referenceText = null,
            Func<string, string?>? linkTarget = null)
        {
            _linkTarget = linkTarget;
            _citationOf = citation;
            _symbolOf = symbol;
            _constants = constants;
            _footnote = footnote;
            _endnote = endnote;
            _styleReference = styleReference;
            _referenceText = referenceText;
            _indexResults = indexFieldDepth;
            _inheritedIndexResults = indexFieldDepth;
        }

        /// <summary>What a <c>STYLEREF</c> quotes, or null when the source cannot answer.</summary>
        private readonly Func<string, string?>? _styleReference;

        /// <summary>
        /// Resolves a relationship id to the address it names, or is null where links are not read —
        /// a header, a footer or a note, whose relationships are their own part's and not the body's.
        /// </summary>
        private readonly Func<string, string?>? _linkTarget;

        /// <summary>The link the text being emitted is inside, or null.</summary>
        private string? _link;

        private readonly List<PageAnchor> _anchors = [];

        /// <summary>The bookmarks the paragraph holds, with the offsets they sit at.</summary>
        internal IReadOnlyList<PageAnchor> Anchors => _anchors;

        /// <summary>What a <c>REF</c> naming a bookmark draws, or null when the source cannot answer.</summary>
        private readonly Func<string, string?>? _referenceText;

        /// <summary>What a <c>FILENAME</c> or <c>TITLE</c> field evaluates to.</summary>
        private readonly ConstantFields _constants;

        /// <summary>How a note of a class and an index is cited, which the source resolves.</summary>
        /// <remarks>
        /// A delegate because the walker is nested but not owned: the numbering comes from the document's
        /// settings, which the source read, and a walker is built per paragraph.
        /// </remarks>
        private readonly Func<bool, int, string> _citationOf;

        /// <summary>How a <c>w:sym</c> resolves to a drawable character and face.</summary>
        private readonly Func<string?, char, (string Text, OpenTypeFace Face, FontReference? Font)?> _symbolOf;

        /// <summary>The face a <c>w:sym</c> resolved to, in force for exactly the character it emits.</summary>
        private (OpenTypeFace Face, FontReference? Font)? _symbolFace;

        /// <summary>How deep a paragraph's element nesting is followed.</summary>
        /// <remarks>
        /// Hyperlinks, content controls, smart tags and change regions all wrap runs and do nest, but a
        /// generated file can nest indefinitely and this recurses on untrusted input.
        /// </remarks>
        private const int MaxDepth = 64;

        private readonly StringBuilder _builder = new();
        private readonly List<StyledRange> _ranges = [];
        private readonly List<NoteAnchor> _notes = [];
        private readonly List<FrameAnchor> _frames = [];
        private int _footnote;
        private int _endnote;
        private XElement? _runProperties;

        /// <summary>How many footnotes this paragraph cited, which advances the source's counter.</summary>
        internal int FootnotesSeen { get; private set; }

        /// <summary>How many endnotes it cited.</summary>
        internal int EndnotesSeen { get; private set; }

        /// <summary>Where in the text the last <c>w:br w:type="page"</c> fell, or −1 for none.</summary>
        private int _pageBreakAt = -1;

        /// <summary>
        /// True when a <c>w:br w:type="page"</c> ended the paragraph, so the <em>next</em> one starts a page.
        /// </summary>
        /// <remarks>
        /// A page break is written at the point in the text where the page ends, and the layout model says
        /// "this paragraph starts a page" — the same shape Writer's <c>BreakType_PAGE_BEFORE</c> has, and
        /// the same shape the DOC and RTF forms state directly. Which paragraph it lands on is decided by
        /// what follows the break rather than by the paragraph boundary: LibreOffice defers the break and
        /// applies it at the next run of text (<c>DomainMapper::lcl_utext</c>, which calls
        /// <c>deferBreak(PAGE_BREAK)</c> for U+000C and inserts <c>BreakType_PAGE_BEFORE</c> into the
        /// <em>current</em> paragraph context on the next text it sees). So a break with text after it in
        /// the same paragraph breaks before that paragraph, and only one with nothing after it carries over.
        /// </remarks>
        internal bool BreaksPage => _pageBreakAt >= 0 && _pageBreakAt >= _builder.Length;

        /// <summary>True when the break fell before this paragraph's own text, so this one starts a page.</summary>
        internal bool BreaksPageHere => _pageBreakAt >= 0 && _pageBreakAt < _builder.Length;

        private bool _inInstruction;

        /// <summary>
        /// How many open fields have had their value written by this walk, so their cached results are
        /// not drawn.
        /// </summary>
        /// <remarks>
        /// A counter rather than a flag because fields nest: a <c>FILENAME</c> inside a <c>HYPERLINK</c>
        /// closes before the link does, and a flag cleared at the inner end would let the outer field's
        /// remaining result through as if it too had been substituted.
        /// </remarks>
        private int _hidden;

        /// <summary>True while nothing the walk reads is drawn.</summary>
        private bool Suppressed => _inInstruction || _hidden > 0;

        /// <summary>The fields whose begin has been seen and whose end has not, innermost last.</summary>
        /// <remarks>
        /// A stack because fields nest and Word writes them nested — a hyperlink around a cross-reference
        /// is two, and a <c>PAGE</c> inside a <c>SEQ</c> is another. Only the fields this walk can
        /// substitute are recorded when they close.
        /// </remarks>
        private readonly Stack<OpenField> _fields = new();

        /// <summary>The page-sensitive fields this paragraph carries, with the spans their results own.</summary>
        internal List<PageFieldSpan> Fields { get; } = [];

        /// <summary>A field between its begin and its end.</summary>
        private sealed class OpenField
        {
            /// <summary>The instruction, accumulated across however many <c>w:instrText</c> carry it.</summary>
            /// <remarks>
            /// Word splits an instruction across runs freely — <c>PAGE  \* MERGE</c> and <c>FORMAT</c> in
            /// two <c>w:instrText</c> is ordinary — so a reader looking at one element at a time sees a
            /// name that is not the field's.
            /// </remarks>
            internal StringBuilder Instruction { get; } = new();

            /// <summary>Where the result began, or −1 for a field with no separator and so no result.</summary>
            internal int ResultAt { get; set; } = -1;

            /// <summary>The address a <c>HYPERLINK</c> field's result links to, or null for any other field.</summary>
            internal string? Link { get; set; }

            /// <summary>The link that was in force when this field's result began, restored at its end.</summary>
            internal string? OuterLink { get; set; }

            /// <summary>
            /// True when this walk wrote the field's value itself, so its cached result is hidden.
            /// </summary>
            internal bool Substituted { get; set; }

            /// <summary>
            /// The run properties in force where the instruction was written.
            /// </summary>
            /// <remarks>
            /// A field with no separator has no result runs to take formatting from, and the run holding
            /// its <c>fldChar end</c> is usually bare — so the instruction's own run is the nearest thing
            /// to what the producer meant the value to look like. Word writes the two with the same
            /// <c>w:rPr</c>, which is what makes this the right guess rather than merely a guess.
            /// </remarks>
            internal XElement? InstructionProperties { get; set; }

            /// <summary>
            /// True when this field is a <c>TOC</c>, <c>INDEX</c> or <c>BIBLIOGRAPHY</c> whose result
            /// is being read, so the runs inside it do not take the character style they name.
            /// </summary>
            internal bool IsIndexResult { get; set; }

            /// <summary>
            /// True when pagination will write this field's value over its cached result, so the
            /// result's runs take <see cref="InstructionProperties"/> rather than their own.
            /// </summary>
            /// <remarks>
            /// See <see cref="RewritesTheResultOf"/> for the rule and what it is measured against.
            /// </remarks>
            internal bool RewritesResult { get; set; }
        }

        /// <summary>
        /// True when this walk replaces the field's value, so the value's character properties are
        /// the <em>field's</em> and not the cached result's.
        /// </summary>
        /// <remarks>
        /// <para>
        /// <strong>26.2.4.2 discards a field result's own <c>w:rPr</c> entirely.</strong> writerfilter
        /// builds a <c>com.sun.star.text.TextField</c> and deletes the cached result text with it, so
        /// the value is drawn in the character properties the field's <em>instruction</em> run carries,
        /// falling back to the paragraph style. Measured on seven one-attribute arms in a footer, where
        /// both engines recompute (<c>probes/fieldrpr-r168</c>): a cached result stating 20 pt red comes
        /// back at the style's 10 pt black in five of them, and the two arms whose <c>w:instrText</c> run
        /// states the 20 pt come back at 20 pt — including the one whose cached result states 10 pt,
        /// which inverts the sign and is what says the rule is the instruction's and not "the smaller of
        /// the two". A <c>w:fldSimple</c> has no instruction run at all, so it falls all the way back to
        /// the paragraph style, and its own <c>w:rPr</c> child does not count.
        /// </para>
        /// <para>
        /// <strong>Word does not, when the field says not to.</strong> <c>\* MERGEFORMAT</c> means
        /// <em>preserve the formatting of the previous result</em>, which is exactly the cached runs'
        /// <c>w:rPr</c>; without the switch Word reformats from the field's own. So the default follows
        /// Word — the rule applies to a field that does not carry the switch — and
        /// <see cref="WordParity"/> applies it to every field, which is 26.2.4.2's own reading. The
        /// reference honours the switch nowhere: five of the seven arms carry it and answer the same as
        /// the one that does not.
        /// </para>
        /// <para>
        /// <strong>Only a field whose value this walk actually writes.</strong> For anything else the
        /// cached text <em>is</em> what gets drawn, and drawing it in anything but its own formatting
        /// would be a defect rather than a fix — the reference recomputes far more fields than this tree
        /// does, and the rule about a value's formatting cannot be borrowed for a value that was never
        /// recomputed.
        /// </para>
        /// <para>
        /// Reach, censused over the corpus DOCX on the properties that change the drawn glyphs rather
        /// than on the presence of a <c>w:rPr</c> — a result stating only <c>w:noProof</c>,
        /// <c>w:webHidden</c> or <c>w:lang</c> is formatted identically either way:
        /// <strong>470 field results in 59 documents</strong> without the switch, which is what moves by
        /// default, and 106 in 29 with it, which move only under <see cref="WordParity"/>.
        /// <c>probes/fieldrpr-r168/census.py</c>.
        /// </para>
        /// </remarks>
        private bool RewritesTheResultOf(string instruction)
            => (FieldInstructions.PageFieldOf(instruction) is not null
                || ValueOf(instruction) is not null)
               && (WordParity.ReproduceLibreOffice
                   || !instruction.Contains("MERGEFORMAT", StringComparison.OrdinalIgnoreCase));

        /// <summary>
        /// How many index fields' results the walk is inside, so a run's <c>w:rStyle</c> is ignored.
        /// </summary>
        /// <remarks>
        /// <para>
        /// A table of contents is written by Word as a field whose cached result is a run of paragraphs,
        /// each entry a <c>w:hyperlink</c> around a run naming the <c>Hyperlink</c> character style — so
        /// taken at its word every entry is blue and underlined. LibreOffice does not take it at its
        /// word: <c>DomainMapper.cxx</c>:3037-3047 looks the style up and then declines to apply it,
        /// <em>"do not add it elements in TOC: they will receive later another style references from
        /// TOC"</em>, so the entries come out in the <c>TOC N</c> paragraph style alone — plain, black,
        /// not underlined. That is what the reference draws, on every document in the corpus that has a
        /// contents list.
        /// </para>
        /// <para>
        /// A counter rather than a flag, and scoped to the field's <em>result</em> rather than to the
        /// whole field, because that is the extent LibreOffice gives it: <c>m_bStartTOC</c> is set when
        /// the field command closes (<c>DomainMapper_Impl.cxx</c>:7643, reached from
        /// <c>CloseFieldCommand</c> at the <c>w:fldChar w:fldCharType="separate"</c>) and cleared only
        /// when that same field's context is popped (<c>DomainMapper_Impl.cxx</c>:9269, guarded by
        /// <c>pContext-&gt;GetTOC().is()</c>), so the <c>PAGEREF</c> fields nested inside each entry do
        /// not end it.
        /// </para>
        /// <para>
        /// <b>And it outlives the paragraph, which is the whole of why the suppression did not work.</b>
        /// A walker is built per paragraph; a <c>TOC</c> field's <c>fldChar begin</c>, its instruction
        /// and its <c>separate</c> sit in the <em>first</em> entry's paragraph and its <c>end</c> in the
        /// last, so a counter reset at every paragraph boundary suppressed the style on entry one and on
        /// nothing else. On <c>150-5370-10H.docx</c> that is one of 43 contents lines. LibreOffice's own
        /// <c>m_bStartTOC</c> is on the document-level import state for exactly this reason, so the depth
        /// is seeded from the source and handed back to it — see
        /// <see cref="DocxLayoutSource._indexFieldDepth"/>.
        /// </para>
        /// </remarks>
        private int _indexResults;

        /// <summary>
        /// How much of <see cref="_indexResults"/> was inherited rather than opened by this paragraph.
        /// </summary>
        /// <remarks>
        /// The field stack is per paragraph, so the <c>fldChar end</c> that closes a multi-paragraph
        /// index field arrives with nothing of that field's on the stack to pop. The inherited depth is
        /// what such an end closes instead — the nested <c>PAGEREF</c> fields inside an entry open and
        /// close within their own paragraph, so an unmatched end is the outer field's and no other.
        /// </remarks>
        private int _inheritedIndexResults;

        /// <summary>How many index fields' results are still open where this paragraph ended.</summary>
        internal int IndexFieldDepth => _indexResults;

        /// <summary>True while the walk is inside an index field's result.</summary>
        private bool InIndexField => _indexResults > 0;

        /// <summary>
        /// Whether an instruction names a field whose result LibreOffice regenerates, and whose runs
        /// therefore lose the character style they name.
        /// </summary>
        /// <remarks>
        /// The three <c>DomainMapper_Impl::IsInTOC</c> covers: <c>handleToc</c>, <c>handleIndex</c> and
        /// <c>handleBibliography</c> all set <c>m_bStartTOC</c>.
        /// </remarks>
        private static bool IsIndexField(string instruction)
            => FieldInstructions.Name(instruction)?.ToUpperInvariant()
                is "TOC" or "INDEX" or "BIBLIOGRAPHY";

        /// <summary>The value a constant field evaluates to, or null when nothing can be computed.</summary>
        private string? ValueOf(string instruction)
        {
            if (FieldInstructions.StyleReferenceName(instruction) is { } style)
            {
                return _styleReference?.Invoke(style) is { Length: > 0 } quoted ? quoted : null;
            }

            // A REF's text is the bookmark's, recomputed on load -- not the {w:fldChar separate}
            // result Word cached. An empty expansion is a legitimate answer for a collapsed bookmark
            // and is *not* taken, because emitting nothing where the cache says something is the one
            // substitution that cannot be checked against the page.
            if (FieldInstructions.ReferenceBookmark(instruction) is { } bookmark)
            {
                return _referenceText?.Invoke(bookmark) is { Length: > 0 } referenced
                    ? referenced
                    : null;
            }

            return FieldInstructions.ConstantFieldOf(instruction) switch
            {
                ConstantField.FileName => _constants.FileName is { Length: > 0 } name ? name : null,
                ConstantField.Title => _constants.Title is { Length: > 0 } title ? title : null,
                _ => null,
            };
        }

        /// <summary>Writes a constant field's value in place of its cached result.</summary>
        /// <param name="field">The field, whose instruction has been read in full.</param>
        /// <param name="properties">The run properties to draw it under, or null for the paragraph's.</param>
        /// <returns>True when a value was written, so the cached result must be hidden.</returns>
        private bool Substitute(OpenField field, XElement? properties)
        {
            if (ValueOf(field.Instruction.ToString()) is not { } value) return false;

            XElement? outer = _runProperties;
            _runProperties = properties;
            Emit(value);
            _runProperties = outer;

            field.Substituted = true;
            _hidden++;
            return true;
        }

        /// <summary>
        /// Makes a result for a page field that has none, so that pagination has somewhere to write.
        /// </summary>
        /// <remarks>
        /// <para>
        /// A complex field with no <c>w:fldChar w:fldCharType="separate"</c> has no cached result at
        /// all, and a page field's value is written by <see cref="PageFields"/> over a span of the
        /// paragraph's text — so with no span there is nothing to write over and the field draws
        /// <em>nothing</em>. 26.2.4.2 computes and draws it. Measured as the <c>NOSEPARATOR</c> arm of
        /// <c>probes/fieldrpr-r168</c>, and censused at <strong>10 such <c>PAGE</c> fields in 8 corpus
        /// documents, every one of them in a running head</strong> — so those eight print no page
        /// number at all, on every page.
        /// </para>
        /// <para>
        /// One character, in the field's own properties, and its <em>width never reaches the page</em>:
        /// <see cref="PageFields.Resolve"/> substitutes before the flow is laid out, precisely so that a
        /// value of a different width from the one it replaces does not move the rest of the line. The
        /// one pass that can draw it is the measuring pass of a two-pass layout, where a
        /// <c>NUMPAGES</c> is not yet resolvable — and there this stands in for a cached result that
        /// does not exist rather than for one it is overriding.
        /// </para>
        /// </remarks>
        private bool PlaceHolder(OpenField field)
        {
            if (FieldInstructions.PageFieldOf(field.Instruction.ToString()) is null) return false;

            XElement? outer = _runProperties;
            _runProperties = field.InstructionProperties;
            Emit("1");
            _runProperties = outer;
            return true;
        }

        /// <summary>
        /// Records a field that has just ended, when it is one whose value pagination decides.
        /// </summary>
        private void CloseField(OpenField field)
        {
            if (field.ResultAt < 0) return;

            if (FieldInstructions.PageFieldOf(field.Instruction.ToString()) is not { } page) return;

            Fields.Add(new PageFieldSpan(
                field.ResultAt, _builder.Length - field.ResultAt, page.Kind, page.Format));
        }

        /// <summary>The paragraph's text, as laid out.</summary>
        internal string Text => _builder.ToString();

        /// <summary>The ranges, in order, partitioning the text.</summary>
        internal IReadOnlyList<StyledRange> Ranges => _ranges;

        /// <summary>The notes referenced in the paragraph, with the offsets their citations occupy.</summary>
        internal List<NoteAnchor> Notes => _notes;

        /// <summary>The floating frames anchored in the paragraph, with the offsets they sit at.</summary>
        internal List<FrameAnchor> Frames => _frames;

        /// <summary>The legacy form checkboxes in the paragraph, with the offsets they sit at.</summary>
        internal List<CheckBoxAnchor> CheckBoxes => _checkBoxes;

        private readonly List<CheckBoxAnchor> _checkBoxes = [];

        /// <summary>Whether a <c>w:checkBox</c> is ticked.</summary>
        /// <remarks>
        /// <c>w:checked</c> is the field's current state and <c>w:default</c> is what it reverts to, so
        /// the first wins where both are present and the second answers where only it is. Both are
        /// OOXML on/off toggles, so a bare element means true.
        /// </remarks>
        private static bool IsChecked(XElement checkBox)
            => Word.Child(checkBox, "checked") is { } current
                ? Word.IsOn(current)
                : Word.Child(checkBox, "default") is { } fallback && Word.IsOn(fallback);

        /// <summary>Walks a <c>w:p</c>.</summary>
        /// <param name="paragraph">The paragraph element.</param>
        /// <param name="citation">
        /// The number a <c>w:footnoteRef</c> in this paragraph stands for, or null when the paragraph is not
        /// a note's. Unlike ODF, a DOCX marks the place its citation goes: the note's own first paragraph
        /// contains a <c>w:footnoteRef</c>, inside a run whose character style is what makes the number
        /// superscript. So the citation is emitted where the file says rather than prepended.
        /// </param>
        internal void Walk(XElement paragraph, string? citation = null)
        {
            _citation = citation;
            Append(paragraph, depth: 0);
        }

        /// <summary>The number a <c>w:footnoteRef</c> stands for, when this paragraph is a note's.</summary>
        private string? _citation;

        /// <summary>Where that number was emitted, or −1 when the paragraph marked no place for one.</summary>
        internal int CitationOffset { get; private set; } = -1;

        private void Append(XElement element, int depth)
        {
            if (depth > MaxDepth) return;

            foreach (XElement child in element.Elements())
            {
                // A formula's own elements share local names with WordprocessingML's — `m:r` and `m:t`
                // against `w:r` and `w:t` — so the switch below reads one as ordinary text unless the
                // namespace is tested first. See `AppendMath`.
                if (child.Name.NamespaceName == OoxmlNamespaces.OfficeMath)
                {
                    // The reference imports the formula into a StarMath object anchored
                    // as-character, so the line takes the object's height rather than the text's.
                    // The anchor is registered here and sized in `MathFrame`; the marks themselves
                    // are still emitted as text, so the formula stays searchable and extractable.
                    if (child.Name.LocalName is "oMath" or "oMathPara")
                    {
                        _frames.Add(new FrameAnchor(_builder.Length, child));
                    }

                    AppendMath(child, depth + 1);
                    continue;
                }

                switch (child.Name.LocalName)
                {
                    case "del" or "delText":
                        // Deleted text is in the file and not on the page.
                        break;

                    case "instrText":
                        // Not drawn — but it is the only statement of what the field computes, so it is
                        // accumulated for the innermost open field rather than merely skipped.
                        if (_fields.Count > 0)
                        {
                            OpenField open = _fields.Peek();
                            open.Instruction.Append(child.Value);
                            open.InstructionProperties ??= _runProperties;
                        }

                        break;

                    case "fldChar":
                        // "separate" ends the instruction and starts the result; "end" closes the field.
                        string? type = Word.Attribute(child, "fldCharType");
                        switch (type)
                        {
                            case "begin":
                                // A legacy form field states what it is on its `begin` marker, in a
                                // `w:ffData`. A checkbox is the one kind that *draws*: Writer makes it a
                                // portion a square of the line's text height wide, so it takes room on
                                // the line as well as putting a box on the page. Recorded before the
                                // instruction opens, because opening it suppresses every emit.
                                if (_hidden == 0
                                    && Word.Child(Word.Child(child, "ffData"), "checkBox")
                                        is { } checkBox)
                                {
                                    _checkBoxes.Add(
                                        new CheckBoxAnchor(
                                            _builder.Length, _runProperties, IsChecked(checkBox)));
                                    Emit(AnchorCharacter.ToString());
                                }

                                _inInstruction = true;
                                if (_fields.Count < MaxDepth) _fields.Push(new OpenField());
                                break;

                            case "separate":
                                _inInstruction = false;
                                if (_fields.Count > 0)
                                {
                                    OpenField open = _fields.Peek();
                                    open.ResultAt = _builder.Length;

                                    // The instruction has been read in full, which is the earliest the
                                    // field's name is known — and it is exactly where LibreOffice
                                    // decides the same thing, in `CloseFieldCommand`.
                                    if (!open.IsIndexResult
                                        && IsIndexField(open.Instruction.ToString()))
                                    {
                                        open.IsIndexResult = true;
                                        _indexResults++;
                                    }

                                    open.RewritesResult =
                                        RewritesTheResultOf(open.Instruction.ToString());

                                    // A HYPERLINK field makes its result a link, the same as the element
                                    // does, until the field's end.
                                    if (FieldInstructions.HyperlinkReference(open.Instruction.ToString()) is { } fieldLink)
                                    {
                                        open.Link = fieldLink;
                                        open.OuterLink = _link;
                                        _link = fieldLink;
                                    }

                                    // The one point at which a field this walk computes can be written:
                                    // the instruction has been read in full, so the field's name is
                                    // known, and the cached result it replaces starts here.
                                    if (_hidden == 0)
                                    {
                                        Substitute(
                                            open,
                                            open.RewritesResult
                                                ? open.InstructionProperties
                                                : _runProperties);
                                    }
                                }

                                break;

                            case "end":
                                _inInstruction = false;

                                // An end with nothing of its own on the stack closes a field that began
                                // in an earlier paragraph, and the only such field this walk tracks is
                                // an index one. See `_inheritedIndexResults`.
                                if (_fields.Count == 0 && _inheritedIndexResults > 0)
                                {
                                    _inheritedIndexResults--;
                                    _indexResults--;
                                }

                                if (_fields.Count > 0)
                                {
                                    OpenField closing = _fields.Pop();

                                    if (closing.Link is not null) _link = closing.OuterLink;

                                    if (closing.IsIndexResult) _indexResults--;

                                    // A field with no separator has no cached result at all, and
                                    // LibreOffice still draws its value — measured on the second
                                    // FILENAME in `CRIF …`'s footer, which the reference draws and this
                                    // walk would otherwise pass over in silence.
                                    if (closing.ResultAt < 0 && _hidden == 0)
                                    {
                                        int at = _builder.Length;
                                        if (Substitute(closing, closing.InstructionProperties))
                                        {
                                            closing.ResultAt = at;
                                            _hidden--;
                                        }
                                        else if (PlaceHolder(closing))
                                        {
                                            closing.ResultAt = at;
                                        }
                                    }
                                    else if (closing.Substituted)
                                    {
                                        _hidden--;
                                    }

                                    CloseField(closing);
                                }

                                break;
                        }

                        break;

                    // The compact form: the instruction is an attribute and the result is the children.
                    // Word writes it for a field with no nested state, which a page number often is.
                    case "fldSimple":
                    {
                        OpenField simple = new() { ResultAt = _builder.Length };
                        simple.Instruction.Append(Word.Attribute(child, "instr") ?? "");

                        // A `w:fldSimple` states no instruction run, so a value written over it takes
                        // the paragraph's own style — see `RewritesTheResultOf`. Where the rule does not
                        // apply, the cached result's first run carries the formatting the producer gave
                        // the field and is kept, which is what `\* MERGEFORMAT` asks for.
                        simple.RewritesResult = RewritesTheResultOf(simple.Instruction.ToString());

                        if (_hidden == 0
                            && Substitute(
                                simple,
                                simple.RewritesResult
                                    ? null
                                    : Word.Child(Word.Child(child, "r"), "rPr")))
                        {
                            _hidden--;
                        }
                        else
                        {
                            // The compact form of an index carries its result the same way and loses
                            // its runs' character styles for the same reason.
                            bool index = IsIndexField(simple.Instruction.ToString());
                            if (index) _indexResults++;

                            // Pushed so that the result's runs can see it: the compact form states no
                            // instruction run, so a value written over it is drawn in the paragraph's
                            // own style, which is what a null `InstructionProperties` means here.
                            bool pushed = simple.RewritesResult && _fields.Count < MaxDepth;
                            if (pushed) _fields.Push(simple);

                            try
                            {
                                Append(child, depth + 1);
                            }
                            finally
                            {
                                if (pushed) _fields.Pop();
                                if (index) _indexResults--;
                            }
                        }

                        CloseField(simple);
                        break;
                    }

                    case "t" when !Suppressed:
                        Emit(child.Value);
                        break;

                    case "tab" when !Suppressed:
                        Emit("\t");
                        break;

                    // A *positional* tab: an absolute stop stated on the element rather than in the
                    // paragraph's `w:tabs`, which is how Word writes a three-part running head with no
                    // tab stops of its own. LibreOffice emits an ordinary tab character for every one
                    // of them — the tokenizer's `CT_PTab` resource is
                    // `<action name="end" action="tab"/>` (`writerfilter/ooxml/model.xml`:18204-18208)
                    // — and only then, for a *left*-aligned one that follows text on the line,
                    // replaces that tab with a line break (`DomainMapper_Impl::HandlePTab`,
                    // `DomainMapper_Impl.cxx`:5550-5604, which returns immediately for every other
                    // alignment). Dropping the element instead runs the parts together: measured on
                    // `PI-doc.-no.-2E-Technical-Review-Report.docx`, whose footer draws
                    // `Page 5 of 12Version 1Last saved …` where the reference spaces the three across
                    // the page.
                    case "ptab" when !Suppressed:
                        if (Word.Attribute(child, "alignment") == "left" && _builder.Length > 0)
                        {
                            Emit(LineSeparator.ToString());
                        }
                        else
                        {
                            Emit("\t");
                        }

                        break;

                    // A character named by slot in a face of its own, and the only run-level element
                    // that overrides the run's font for one character. LibreOffice sets
                    // PROP_CHAR_FONT_NAME to `w:font` with charset SYMBOL and appends the raw `w:char`
                    // (`DomainMapper::sprmWithProps`, `LN_EG_RunInnerContent_sym`), so the face travels
                    // with the character rather than with the run.
                    case "sym" when !Suppressed:
                        EmitSymbol(child);
                        break;

                    // Word states it as an element rather than a character, and dropping it closes up
                    // the space it occupies exactly as dropping a `w:tab` would.
                    //
                    // An ordinary hyphen, not U+2011, and that is measured rather than assumed. The
                    // import carries it as U+2011 (`OOXMLFastContextHandler.cxx:54`, `uNoBreakHyphen`)
                    // and the *layout* then swaps the character out:
                    // `case CHAR_HARDHYPHEN: pPor = new SwBlankPortion('-')`
                    // (`sw/source/core/text/itrform2.cxx:1881-1882`). The reference PDF agrees — the
                    // text layer of `Company-profile-2022-EN.docx` reads `the -600 series` with a
                    // U+002D — and it has to, because U+2011 is in neither Carlito nor any Liberation
                    // face, so keeping it would draw a fallback face's glyph beside text in neither.
                    //
                    // What this does not reproduce is the half of the name that says "no break": a
                    // `SwBlankPortion` cannot be broken and a U+002D is UAX #14 class HY, which is a
                    // break opportunity. Drawing the hyphen in the right face is worth more than the
                    // breaking, which only differs when a line ends exactly there.
                    case "noBreakHyphen" when !Suppressed:
                        Emit("-");
                        break;

                    // A `w:br` is three things wearing one name and only one of them is a line break.
                    // `w:type="page"` moves everything after it to the next page and contributes no
                    // character at all: LibreOffice turns it back into the DOC's own U+000C
                    // (`OOXMLBreakHandler::~OOXMLBreakHandler`, `writerfilter/ooxml/Handler.cxx:246`)
                    // and then *defers* it, applying it to the paragraph that follows as
                    // `BreakType_PAGE_BEFORE` (`dmapper/DomainMapper.cxx:4379`).
                    case "br" when !Suppressed:
                        if (Word.Attribute(child, "type") == "page") _pageBreakAt = _builder.Length;
                        else Emit(LineSeparator.ToString());
                        break;

                    case "footnoteReference" or "endnoteReference":
                    {
                        // A note reference carries its citation, which Word draws in the sentence as a
                        // superscript and again at the head of the note. The style comes from the run this
                        // reference sits in, which is what carries w:vertAlign="superscript".
                        bool isEndnote = Word.Is(child, "endnoteReference");
                        string number = _citationOf(isEndnote, isEndnote ? _endnote : _footnote);

                        _notes.Add(new NoteAnchor(
                            _builder.Length, Word.Attribute(child, "id"), isEndnote, number));

                        _inCitation = true;
                        Emit(number);
                        _inCitation = false;

                        if (isEndnote)
                        {
                            _endnote++;
                            EndnotesSeen++;
                        }
                        else
                        {
                            _footnote++;
                            FootnotesSeen++;
                        }

                        break;
                    }

                    case "footnoteRef" or "endnoteRef":
                        // The note's own citation, at the place the file marks for it. Marked as a citation
                        // so that it falls back to superscript, because the style that should supply it
                        // usually does not: LibreOffice exports its built-in `Footnote Characters` as an
                        // *empty* w:rPr and relies on the importer knowing what that style is. A reader
                        // taking the file at its word draws the number full size on the baseline, where it
                        // fuses with the note's first word.
                        if (_citation is not null)
                        {
                            CitationOffset = _builder.Length;
                            _inCitation = true;
                            Emit(_citation);
                            _inCitation = false;
                        }

                        break;

                    // A floating frame occupies a *position* in the paragraph and is not part of it: its
                    // own text belongs to a rectangle of its own, and it puts no character in the text.
                    // Writer says the same in one place — `SwFormatFlyCnt`, and therefore the
                    // `CH_TXTATR_BREAKWORD` that `GetCharOfTextAttr` gives `RES_TXTATR_FLYCNT`
                    // (`sw/source/core/txtnode/thints.cxx`:3633-3652), is inserted for `FLY_AS_CHAR`
                    // alone (`SwDoc::SetFlyFrameAnchor`, `sw/source/core/doc/docfly.cxx`:337-348). An
                    // `FLY_AT_CHAR` fly carries a `SwPosition` on its `SwFormatAnchor` and nothing in
                    // the node's text.
                    //
                    // <b>A character here is a break opportunity, and that is what it costs.</b> An
                    // inline object is measured at a *boundary* — see `MeasuredParagraph`, where an
                    // object widens every prefix past its offset — so a picture wider than the measure
                    // is forced onto the line it starts when nothing precedes it, and breaks onto the
                    // next line when something does. A run of anchor characters is "something". Measured
                    // on `PES-Technical-Report-Template_Jan_2019.docx`, whose cover paragraph is four
                    // `wp:anchor` runs followed by one `wp:inline` picture 612.5 pt wide on a 612 pt
                    // zero-margin page: with the characters emitted the picture took a second line and
                    // therefore a second page, and the reference's fourteen pages ran one behind ours
                    // from page 2 to page 13. Bisected by deleting runs — the picture alone is right,
                    // the picture with *any one* anchor before it is wrong, and moving the picture run
                    // to the head of the paragraph is right again.
                    //
                    // The offset recorded is unchanged in meaning: it is where the anchor sits, and a
                    // control character was zero-width anyway (`ShapingControls`), so an at-character
                    // frame's position along its line is the same width of text as before.
                    //
                    // It also settles the height question `HoldsOnlyFloatingFrames` used to answer: a
                    // paragraph whose whole content is floating drawings now *is* empty, so its mark
                    // sizes it. That is the rule `088_Printable_Graph_Paper_Template_Quality_layout`
                    // needs — its last paragraph is a 2 pt mark holding one anchored logo, and read at
                    // the 11 pt body size it overflows and costs the document a page.
                    case "drawing":
                        _frames.Add(new FrameAnchor(_builder.Length, child));
                        if (!DocxFrames.IsFloating(child)) Emit(AnchorCharacter.ToString());
                        break;

                    // A `w:pict` or a `w:object` is a VML shape. It states its size in a CSS `style`
                    // rather than in a `wp:extent`, and only an inline one takes room on the line —
                    // see `DocxVmlFrames`, which decides both and returns nothing for a floating
                    // shape, leaving the anchor character to stand for it as it did before.
                    case "pict" or "object":
                        _frames.Add(new FrameAnchor(_builder.Length, child));
                        Emit(AnchorCharacter.ToString());
                        break;

                    case "commentReference":
                        Emit(AnchorCharacter.ToString());
                        break;

                    // A bookmark is a named place, and what an internal link, a cross-reference or a
                    // table-of-contents entry points at. Nothing is drawn for it; its offset is kept so a
                    // PDF can declare a destination there.
                    case "bookmarkStart":
                        if (_hidden == 0 && Word.Attribute(child, "name") is { Length: > 0 } bookmark)
                        {
                            _anchors.Add(new PageAnchor(_builder.Length, bookmark));
                        }

                        break;

                    case "pPr" or "bookmarkEnd" or "proofErr" or "rPr":
                        break;

                    // A link around runs. The runs keep their own formatting — a hyperlink is usually
                    // blue and underlined because its *character style* says so, not because of this
                    // element — and the link is one more property of the text they emit.
                    case "hyperlink":
                    {
                        string? outerLink = _link;
                        if (HyperlinkAddress(child) is { } address) _link = address;

                        try
                        {
                            Append(child, depth + 1);
                        }
                        finally
                        {
                            _link = outerLink;
                        }

                        break;
                    }

                    case "r":
                        // The one element that carries character formatting. Runs do not nest, but this
                        // saves and restores anyway so that a malformed file cannot lose the outer state.
                        //
                        // Inside the cached result of a field whose value pagination will write over it,
                        // the run's own `w:rPr` is not what the value is drawn in — see
                        // `RewritesTheResultOf`. The field's own is substituted for it, and null there is
                        // an answer rather than an absence: it means the paragraph's style, which is what
                        // a `w:fldSimple` falls back to.
                        XElement? outer = _runProperties;
                        _runProperties =
                            _fields.Count > 0 && _fields.Peek() is { RewritesResult: true, ResultAt: >= 0 } rewritten
                                ? rewritten.InstructionProperties
                                : Word.Child(child, "rPr");
                        Append(child, depth + 1);
                        _runProperties = outer;
                        break;

                    default:
                        Append(child, depth + 1);
                        break;
                }
            }
        }

        /// <summary>
        /// Where a <c>w:hyperlink</c> leads: an address, <c>#name</c> for a bookmark, or null for a link
        /// that states neither.
        /// </summary>
        /// <remarks>
        /// The same rule the extraction side applies: an external link is a relationship, an internal one
        /// names a bookmark in <c>w:anchor</c>, and an address with an anchor is the address with a
        /// fragment. A relationship id this walk cannot resolve (see <see cref="_linkTarget"/>) leaves a
        /// link that states an anchor an internal one, and one that does not no link.
        /// </remarks>
        private string? HyperlinkAddress(XElement hyperlink)
        {
            string? anchor = Word.Attribute(hyperlink, "anchor") is { Length: > 0 } named ? named : null;
            string? target = Word.RelationshipId(hyperlink) is { Length: > 0 } id ? _linkTarget?.Invoke(id) : null;

            if (target is { Length: > 0 }) return anchor is null ? target : target + "#" + anchor;
            return anchor is null ? null : "#" + anchor;
        }

        /// <summary>
        /// Emits a <c>w:sym</c>: one character, in the face the element names rather than the run's.
        /// </summary>
        /// <remarks>
        /// The slot is passed on exactly as the file states it — <c>00DE</c> stays <c>U+00DE</c> and
        /// <c>F0B7</c> stays <c>U+F0B7</c> — because the recode tables accept a symbol slot in both its
        /// plain and its Private Use Area spelling and LibreOffice hands them the value unaltered.
        /// Nothing is emitted when the resolver declines: the code point means nothing outside the face
        /// that is missing, so drawing it in the paragraph's own face would put a <c>.notdef</c> box
        /// where the document asked for a picture.
        /// </remarks>
        private void EmitSymbol(XElement symbol)
        {
            string? code = Word.Attribute(symbol, "char");
            if (code is null) return;
            if (!ushort.TryParse(code, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out ushort slot))
                return;

            if (_symbolOf(Word.Attribute(symbol, "font"), (char)slot) is not { } resolved) return;

            _symbolFace = (resolved.Face, resolved.Font);
            Emit(resolved.Text);
            _symbolFace = null;
        }

        /// <summary>
        /// Walks a formula, emitting the characters it states and the characters it generates.
        /// </summary>
        /// <remarks>
        /// <para>
        /// OMML states a formula structurally: a fraction is a <c>m:f</c> holding a numerator and a
        /// denominator and nothing says "draw a rule between them", a radical is a <c>m:rad</c> and
        /// nothing says <c>U+221A</c>, a delimiter pair is a <c>m:d</c> whose brackets live in
        /// <em>attributes</em>. So a reader that emits only the <c>m:t</c> leaves draws
        /// <c>a1b2</c> for a fraction of two subscripted terms and <c>i=1nxi</c> for a sum. Measured
        /// against 26.2.4.2 on a fifteen-construct ladder
        /// (<c>tests/corpus/features/words-formula-complexity.docx</c>): fifteen of fifteen wrong, and
        /// the page's alphanumeric count identical on both sides throughout, so no gate column could
        /// see it.
        /// </para>
        /// <para>
        /// <strong>This is the one-dimensional half.</strong> The reference builds a StarMath object per
        /// <c>m:oMath</c> and lays it out in two dimensions — a stacked fraction, limits above and below
        /// an n-ary, a vinculum over a radicand. Reproducing that needs a box model with its own
        /// baseline, which this does not have; what it does is put every generated mark on the line in
        /// reading order, so the formula is legible, searchable and roughly the right width. The
        /// vertical half is the height reservation, which gives the line the reference's own height for
        /// the line even though we draw the content flat.
        /// </para>
        /// <para>
        /// The defaults are the specification's and each costs a formula when it is missed: a
        /// <c>m:d</c> with no <c>m:begChr</c> is a parenthesis rather than nothing, an <c>m:nary</c>
        /// with no <c>m:chr</c> is an integral rather than nothing.
        /// </para>
        /// </remarks>
        /// <param name="element">A node of the formula's subtree.</param>
        /// <param name="depth">Recursion depth, against the same bound the ordinary walk uses.</param>
        private void AppendMath(XElement element, int depth)
        {
            if (depth > MaxDepth) return;

            switch (element.Name.LocalName)
            {
                // Properties and the control run that carries a construct's own formatting. Neither is
                // drawn, and `m:ctrlPr` holds a `w:rPr` that would otherwise be read as a run's.
                case "argPr" or "ctrlPr" or "accPr" or "barPr" or "boxPr" or "dPr" or "eqArrPr"
                    or "fPr" or "funcPr" or "groupChrPr" or "limLowPr" or "limUppPr" or "mPr"
                    or "mcPr" or "mcs" or "mc" or "naryPr" or "phantPr" or "radPr" or "sPrePr"
                    or "sSubPr" or "sSubSupPr" or "sSupPr" or "oMathParaPr":
                    return;

                case "t":
                    if (!Suppressed) Emit(element.Value);
                    return;

                case "r":
                {
                    // `m:r` carries a `w:rPr` exactly as `w:r` does — that half really is
                    // WordprocessingML — so the run formatting in force is saved and restored the same
                    // way. Its `m:t` children are reached by the recursion below.
                    XElement? outer = _runProperties;
                    if (Word.Child(element, "rPr") is { } properties) _runProperties = properties;
                    AppendMathChildren(element, depth);
                    _runProperties = outer;
                    return;
                }

                case "sSub":
                    AppendMathChild(element, "e", depth);
                    AppendScript(element, "sub", subscript: true, depth);
                    return;

                case "sSup":
                    AppendMathChild(element, "e", depth);
                    AppendScript(element, "sup", subscript: false, depth);
                    return;

                case "sSubSup":
                    AppendMathChild(element, "e", depth);
                    AppendScript(element, "sub", subscript: true, depth);
                    AppendScript(element, "sup", subscript: false, depth);
                    return;

                case "sPre":
                    // A pre-script states its scripts before the base and draws them to its left.
                    AppendScript(element, "sub", subscript: true, depth);
                    AppendScript(element, "sup", subscript: false, depth);
                    AppendMathChild(element, "e", depth);
                    return;

                case "f":
                    // Until there is a box model to stack them in, a fraction reads as a solidus. The
                    // *height* the reference gives it is reserved regardless — see `DocxMathHeight`.
                    AppendMathChild(element, "num", depth);
                    Emit("/");
                    AppendMathChild(element, "den", depth);
                    return;

                case "rad":
                {
                    if (!MathFlag(MathChild(element, "radPr"), "degHide"))
                    {
                        AppendScript(element, "deg", subscript: false, depth);
                    }

                    Emit("√");
                    AppendMathChild(element, "e", depth);
                    return;
                }

                case "nary":
                {
                    XElement? properties = MathChild(element, "naryPr");
                    Emit(MathValue(properties, "chr") is { Length: > 0 } sign ? sign : "\u222B");
                    if (!MathFlag(properties, "subHide"))
                    {
                        AppendScript(element, "sub", subscript: true, depth);
                    }

                    if (!MathFlag(properties, "supHide"))
                    {
                        AppendScript(element, "sup", subscript: false, depth);
                    }

                    AppendMathChild(element, "e", depth);
                    return;
                }

                case "d":
                {
                    // The brackets are attributes rather than elements, and their defaults are real: a
                    // `m:d` stating nothing is a parenthesised group, not a bare one.
                    XElement? properties = MathChild(element, "dPr");
                    Emit(Delimiter(properties, "begChr", "("));

                    bool first = true;
                    foreach (XElement argument in element.Elements())
                    {
                        if (argument.Name.LocalName != "e") continue;
                        if (!first) Emit(Delimiter(properties, "sepChr", "|"));
                        AppendMath(argument, depth + 1);
                        first = false;
                    }

                    Emit(Delimiter(properties, "endChr", ")"));
                    return;
                }

                case "func":
                    AppendMathChild(element, "fName", depth);
                    AppendMathChild(element, "e", depth);
                    return;

                case "acc":
                    // The accent follows its base as a combining mark, which is where Unicode puts it.
                    AppendMathChild(element, "e", depth);
                    Emit(MathValue(MathChild(element, "accPr"), "chr") is { Length: > 0 } accent
                        ? accent
                        : "̂");
                    return;

                case "limLow":
                    AppendMathChild(element, "e", depth);
                    AppendScript(element, "lim", subscript: true, depth);
                    return;

                case "limUpp":
                    AppendMathChild(element, "e", depth);
                    AppendScript(element, "lim", subscript: false, depth);
                    return;

                case "m":
                    // A matrix reads row by row: cells separated by a space, rows by a line break, which
                    // is what a one-dimensional rendering of a grid can honestly offer.
                    foreach (XElement row in element.Elements())
                    {
                        if (row.Name.LocalName != "mr") continue;

                        bool opening = true;
                        foreach (XElement cell in row.Elements())
                        {
                            if (cell.Name.LocalName != "e") continue;
                            if (!opening) Emit(" ");
                            AppendMath(cell, depth + 1);
                            opening = false;
                        }

                        Emit("\n");
                    }

                    return;

                default:
                    AppendMathChildren(element, depth);
                    return;
            }
        }

        /// <summary>Walks a math element's children, staying inside the math walk.</summary>
        private void AppendMathChildren(XElement element, int depth)
        {
            foreach (XElement child in element.Elements())
            {
                // A `w:` element inside a formula — the `w:rPr` of an `m:r` is the common one — belongs
                // to the ordinary walk, which is what reads run formatting.
                if (child.Name.NamespaceName == OoxmlNamespaces.OfficeMath) AppendMath(child, depth + 1);
            }
        }

        /// <summary>Walks one named child of a math element, if it has one.</summary>
        private void AppendMathChild(XElement element, string name, int depth)
        {
            foreach (XElement child in element.Elements())
            {
                if (child.Name.LocalName == name
                    && child.Name.NamespaceName == OoxmlNamespaces.OfficeMath)
                {
                    AppendMath(child, depth + 1);
                    return;
                }
            }
        }

        /// <summary>
        /// Walks a script — a subscript, a superscript, a radical's degree or an n-ary's limit — with the
        /// run properties a raised or lowered run would carry.
        /// </summary>
        /// <remarks>
        /// A synthesised <c>w:rPr</c> is the carrier because it is what the rest of this reader already
        /// understands: <see cref="WordCharacterFormat"/> turns <c>w:vertAlign</c> into the same
        /// escapement an ordinary superscript gets, so the script is drawn smaller and off the baseline
        /// without a second formatting path existing to disagree with the first.
        /// <para>
        /// It is not StarMath's own arithmetic, and the residual is measured rather than guessed. The
        /// reference sets a script at 60 % (<c>SIZ_INDEX</c>) and drops it by
        /// <c>max(0.20·em, ascent·0.60·em − 0.30·em)</c> — the second arm, a clamp to the delimiter line
        /// in <c>SmSubSupNode::Arrange</c>, winning for every Latin face — where an ordinary
        /// <c>w:vertAlign</c> subscript is 58 % and a fixed fraction of the font's height. The gap is a
        /// fraction of a point at body size and the structure is right; closing it needs the script's
        /// own escapement computed from the face, which this does not yet do.
        /// </para>
        /// </remarks>
        /// <param name="element">The construct holding the script.</param>
        /// <param name="name">Which child of it is the script.</param>
        /// <param name="subscript">True to lower it, false to raise it.</param>
        /// <param name="depth">Recursion depth.</param>
        private void AppendScript(XElement element, string name, bool subscript, int depth)
        {
            XElement? script = null;
            foreach (XElement child in element.Elements())
            {
                if (child.Name.LocalName == name
                    && child.Name.NamespaceName == OoxmlNamespaces.OfficeMath)
                {
                    script = child;
                    break;
                }
            }

            if (script is null) return;

            XElement? outer = _runProperties;
            _runProperties = ScriptProperties(outer, subscript);
            AppendMath(script, depth + 1);
            _runProperties = outer;
        }

        /// <summary>
        /// The run properties in force, with a vertical alignment added.
        /// </summary>
        /// <remarks>
        /// The outer properties are copied rather than replaced so a script inside a bold or coloured
        /// run keeps that, and the result is cached against the pair so that two scripts under one run
        /// share an element — <see cref="Emit"/> merges adjacent ranges by reference equality of their
        /// properties, so building a fresh element per script would split every one of them into its
        /// own range for nothing.
        /// </remarks>
        private XElement ScriptProperties(XElement? outer, bool subscript)
        {
            Dictionary<(XElement?, bool), XElement> cache = _scriptProperties;
            if (cache.TryGetValue((outer, subscript), out XElement? cached)) return cached;

            XElement properties = outer is null
                ? new XElement(XName.Get("rPr", OoxmlNamespaces.WordprocessingML))
                : new XElement(outer);

            properties.Elements(XName.Get("vertAlign", OoxmlNamespaces.WordprocessingML))
                .Remove();
            properties.Add(new XElement(
                XName.Get("vertAlign", OoxmlNamespaces.WordprocessingML),
                new XAttribute(
                    XName.Get("val", OoxmlNamespaces.WordprocessingML),
                    subscript ? "subscript" : "superscript")));

            cache[(outer, subscript)] = properties;
            return properties;
        }

        /// <summary>One synthesised script `w:rPr` per (outer properties, direction) pair.</summary>
        private readonly Dictionary<(XElement?, bool), XElement> _scriptProperties = [];

        /// <summary>A delimiter character stated on <c>m:dPr</c>, or the specification's default.</summary>
        private static string Delimiter(XElement? properties, string name, string fallback)
            => MathValue(properties, name) is { Length: > 0 } stated ? stated : fallback;

        /// <summary>
        /// The <c>m:val</c> of a formula property's named child, or null.
        /// </summary>
        /// <remarks>
        /// <strong>Not <see cref="Word.Value"/>.</strong> That resolves the child and the attribute in
        /// the WordprocessingML namespace, and a formula states both in its own: <c>&lt;m:chr
        /// m:val="∑"/&gt;</c>. Asking the `w:` question of an `m:` element does not fail — it answers
        /// null, which every caller here reads as "not stated" and replaces with a default. Measured:
        /// the ladder fixture's summation drew <c>∫</c>, because `m:naryPr/m:chr` was invisible and the
        /// n-ary default is the integral.
        /// </remarks>
        private static string? MathValue(XElement? element, string name)
        {
            if (element is null) return null;

            XElement? child = element.Element(XName.Get(name, OoxmlNamespaces.OfficeMath));
            return child?.Attribute(XName.Get("val", OoxmlNamespaces.OfficeMath))?.Value;
        }

        /// <summary>A formula element's named child, in the math namespace.</summary>
        private static XElement? MathChild(XElement? element, string name)
            => element?.Element(XName.Get(name, OoxmlNamespaces.OfficeMath));

        /// <summary>True when a formula property's named flag is on.</summary>
        /// <remarks>
        /// An OMML boolean is on when it is present and its <c>m:val</c> is not a false token — the
        /// same three-valued shape <c>w:val</c> has, so a bare <c>&lt;m:degHide/&gt;</c> means hidden.
        /// </remarks>
        private static bool MathFlag(XElement? element, string name)
        {
            if (element is null) return false;

            XElement? child = element.Element(XName.Get(name, OoxmlNamespaces.OfficeMath));
            if (child is null) return false;

            string? value = child.Attribute(XName.Get("val", OoxmlNamespaces.OfficeMath))?.Value;
            return value is not ("0" or "false" or "off");
        }

        /// <summary>Appends text under the run properties currently in force.</summary>
        private void Emit(string text)
        {
            if (text.Length == 0) return;

            _builder.Append(text);

            // Adjacent runs with the same properties merge, which matters because a DOCX splits runs for
            // reasons that are not formatting: a proofing error, a revision id, a bookmark boundary.
            // A symbol never merges: its face is its own and the character beside it is not in it.
            if (_ranges.Count > 0
                && _symbolFace is null
                && _ranges[^1].Symbol is null
                && _ranges[^1].IsCitation == _inCitation
                && _ranges[^1].InIndexField == InIndexField
                && string.Equals(_ranges[^1].Link, _link, StringComparison.Ordinal)
                && _ranges[^1].RunProperties == _runProperties)
            {
                _ranges[^1] = _ranges[^1] with { Length = _ranges[^1].Length + text.Length };
                return;
            }

            _ranges.Add(new StyledRange(
                _builder.Length - text.Length, text.Length, _runProperties, _inCitation, _symbolFace,
                InIndexField, _link));
        }

        /// <summary>True while a note's citation is being emitted.</summary>
        private bool _inCitation;
    }

    /// <summary>
    /// A citation's style, defaulted to superscript when the run does not say so.
    /// </summary>
    /// <remarks>
    /// Word's own <c>FootnoteReference</c> character style sets <c>w:vertAlign="superscript"</c>, and a
    /// document that has it is read correctly without this. LibreOffice's DOCX export does not always write
    /// it, and a document whose notes were added by something else may not either — so the default matches
    /// what Word draws rather than what the file happens to state. Applied only when nothing has been said,
    /// so a run that does state a shift keeps it.
    /// </remarks>
    private static WordTextStyle AsCitation(WordTextStyle style)
        => style.Escapement.IsNone
            ? style with { Escapement = Layout.Escapement.Superscript }
            : style;

    /// <summary>
    /// How a note of each class is cited, which is not the same for the two.
    /// </summary>
    /// <remarks>
    /// Two sequences in two formats, from the document's <c>w:footnotePr</c> and <c>w:endnotePr</c> where it
    /// has them and from LibreOffice's own defaults where it does not — footnotes 1, 2, 3 and endnotes
    /// i, ii, iii, which is measured rather than assumed.
    /// </remarks>
    /// <param name="isEndnote">True for an endnote.</param>
    /// <param name="index">How many notes of the class came before, counted from zero.</param>
    private string CitationOf(bool isEndnote, int index)
        => (isEndnote ? _endnoteNumbering : _footnoteNumbering).Citation(index);

    /// <summary>
    /// What <c>w:beforeAutospacing</c> and <c>w:afterAutospacing</c> stand for in this document.
    /// </summary>
    /// <remarks>
    /// Three answers, in LibreOffice's own order of precedence
    /// (<c>DomainMapper.cxx</c>:916-953). <c>w:doNotUseHTMLParagraphAutoSpacing</c> wins outright and
    /// means five points; otherwise a document saved in <strong>web view</strong> gets 49 twips and
    /// every other document 280. The web branch is easy to miss because <c>w:view</c> is not a
    /// compatibility flag — it sits directly under <c>w:settings</c>, beside the zoom and the ruler,
    /// and reads like a preference rather than a measurement.
    /// </remarks>
    private static Length AutoSpacing(XElement? settings, WordCompatibility compatibility)
    {
        if (compatibility.DoNotUseHtmlParagraphAutoSpacing) return WordParagraphFormats.WordAutoSpacing;

        return Word.Attribute(Word.Child(settings, "view"), "val") == "web"
            ? WordParagraphFormats.WebAutoSpacing
            : WordParagraphFormats.HtmlAutoSpacing;
    }

    /// <summary>
    /// The document's default tab interval.
    /// </summary>
    /// <remarks>
    /// Half an inch when the document does not say, which is what Word uses. A zero would make a tab
    /// advance nowhere, so it is treated as absent rather than honoured.
    /// </remarks>
    private static Length TabInterval(XElement? settings)
        => Word.Attribute(Word.Child(settings, "defaultTabStop"), "val") is { } text
           && Word.Long(text, out long twips)
           && twips > 0
            ? Length.FromTwips(twips)
            : Length.FromTwips(720);

    /// <summary>
    /// Which version of Word wrote the file, from <c>w:compat</c>, or <c>-1</c> when it does not say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A single number covering a decade of behaviour changes: 12 is Word 2007, 14 is 2010 and 15 is 2013
    /// and after. Word writes it as a <c>w:compatSetting</c> named <c>compatibilityMode</c> in the
    /// Microsoft namespace, which is a URI rather than the <c>w:</c> one — so the name and the URI both
    /// have to match, and a setting from another vendor's namespace is not this one.
    /// </para>
    /// <para>
    /// Absent stays <c>-1</c> rather than defaulting to 12, following
    /// <c>SettingsTable::GetWordCompatibilityMode</c>: everything that consults it asks whether the mode is
    /// <em>below</em> 15, and −1 is, so a file that says nothing gets the older behaviour without the
    /// reader having to invent a version for it.
    /// </para>
    /// </remarks>
    private static int CompatibilityMode(XElement? settings)
    {
        const string wordUri = "http://schemas.microsoft.com/office/word";

        foreach (XElement setting in Word.Children(Word.Child(settings, "compat"), "compatSetting"))
        {
            if (Word.Attribute(setting, "name") != "compatibilityMode") continue;
            if (Word.Attribute(setting, "uri") != wordUri) continue;

            if (int.TryParse(
                    Word.Attribute(setting, "val"), NumberStyles.Integer, CultureInfo.InvariantCulture,
                    out int mode))
            {
                return mode;
            }
        }

        return -1;
    }

    /// <summary>
    /// The face a VML <c>v:textpath</c> is set in, from the family its CSS names.
    /// </summary>
    /// <remarks>
    /// Its text is an attribute rather than a run, so it never reaches the walk that resolves every
    /// other face in the document, and the reader has to be handed one. Resolved through the same
    /// <see cref="Face(WordTextStyle)"/> the body uses so that a family the document also sets text
    /// in comes out the same face — <c>vmlformatting.cxx:1026-1028</c> sets it as a plain
    /// <c>CharFontName</c>, which is what a run states too.
    /// </remarks>
    private OpenTypeFace? VmlFace(string? family)
        => Face(new WordTextStyle(family, Length.Zero, 400, false, null));

    private OpenTypeFace? Face(WordTextStyle text)
    {
        (string? Family, int Weight, bool Italic, FontFamilyClass Class, WriterScript Script,
            string? Language) key = text.FaceKey;
        if (_faces.TryGetValue(key, out OpenTypeFace? cached)) return cached;

        OpenTypeFace? face = null;
        try
        {
            // The declared family only. The table declares a pitch too and LibreOffice's DOCX filter
            // does not act on it: probed on 26.2.4.2 with a one-run package, `Garamond` declared
            // `swiss` moves the fallback from DejaVu Serif to DejaVu Sans while `Garamond` declared
            // `fixed` — and `MS Mincho` declared `modern`+`fixed` — leaves it exactly where it was.
            // Its ODF filter *does* honour `style:font-pitch`, so this is a difference between the
            // two importers rather than a property of the resolver, and passing the pitch here put
            // one corpus document into DejaVu Sans Mono that the reference sets in DejaVu Sans.
            //
            // And a family the table says nothing about is not undeclared as far as the *filter* is
            // concerned: it takes Writer's roman default, which is why an unrecognised family drawn
            // through this filter comes out DejaVu Serif where the same name through the ODF filter
            // comes out whatever fontconfig files it under. See `WordFallbackClass`, and
            // `probes/words-r54/font-fallback-rule.py` for the 98 files that measure it.
            FontFamilyClass declared = WordFallbackClass.ForScript(
                text.FamilyName, text.DeclaredClass, text.Script);

            FontReference reference = _fonts.Resolve(
                new FontRequest(
                    text.FamilyName ?? string.Empty, text.Weight, text.IsItalic,
                    DeclaredClass: declared, Language: text.ItemLanguage));

            face = _fonts.LoadOpenType(reference);
            _references[key] = reference;
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // Nothing to measure the paragraph with. Dropping it gives a shorter document rather than
            // an exception out of the middle of a layout.
        }

        _faces[key] = face;
        return face;
    }
}
