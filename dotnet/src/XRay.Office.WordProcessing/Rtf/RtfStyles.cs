using XRay.Office.Text.Layout;
using XRay.Office.Text.Fonts;

namespace XRay.Office.WordProcessing.Rtf;

/// <summary>
/// The formatting one <c>{\stylesheet}</c> entry states, as read off the entry's own group.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The character half resolves through the whole <c>\sbasedon</c> chain; the paragraph half
/// is three properties and reaches a paragraph only from the chain <em>above</em> the style it
/// names.</strong> That asymmetry is measured — 53 one-page probes in
/// <c>probes/rtf-resid-r80/genmatrix.py</c>, thirteen paragraph properties against 26.2.4.2 — and it
/// is not a quirk of the corpus: it is <c>cloneAndDeduplicateSprm</c>
/// (<c>sw/source/writerfilter/rtftok/rtfsprm.cxx</c>:290-340), whose <em>"not found - try to
/// override style with default"</em> branch writes <c>getDefaultSPRM</c>'s value onto the paragraph
/// as direct formatting for every paragraph property the <em>named</em> style states and the
/// paragraph does not. So the named style's own <c>\sb \sa \li \ri \fi \sl</c> are each reset to
/// RTF's default before they can reach the text, and an ancestor's are not, because
/// <c>lcl_copyFlatten</c> (<c>rtfdocumentimpl.cxx</c>:490-514) flattens only the entry's own
/// <c>pPr</c> into the set that branch walks.
/// </para>
/// <para>
/// Which properties survive follows from the same function's table, and the probes agree with it
/// property for property. <c>getDefaultSPRM</c> (<c>rtfsprm.cxx</c>:154-224) answers a default for
/// <c>ind</c>'s three children and for <c>spacing</c>'s <em>after</em> alone — the
/// <c>LN_CT_PPrBase_spacing</c> case returns a value carrying <c>after = 0</c> and nothing else, and
/// it is taken before the per-attribute recursion, so <em>before</em> is never visited. It answers
/// nothing for <c>jc</c>, <c>keepNext</c> or <c>tabs</c>. Measured, one property per probe:
/// <c>\qc \qr \qj</c> and <c>\sb</c> reach the paragraph from an ancestor and
/// <c>\sa \li \ri \fi \sl</c> reach it from nowhere; <c>\keepn</c> reaches it from the named style
/// as well, which is why it is the one member here resolved over the whole chain.
/// </para>
/// <para>
/// <strong>The earlier reading — "character formatting only" — was right about the effect and wrong
/// about the cause, and the cause is what decides which properties to carry.</strong> It attributed
/// the whole of it to <c>StyleSheetTable::ConvertStyleName</c>
/// (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx</c>:1620-1660) mapping an RTF style
/// <em>name</em> onto Writer's built-in style of that name, so that a pool style's own vertical
/// spacing beat the <c>\sbasedon</c> chain. That mapping is real, but it is not what suppresses the
/// spacing: a style called <c>Centered</c> maps onto no pool style at all and its own <c>\sb480</c>
/// is dropped just the same, while the identical <c>\sb480</c> one level up is honoured.
/// </para>
/// <para>
/// Every member is nullable, and that is the whole point of the type: RTF writes a style as the
/// <em>difference</em> from its <c>\sbasedon</c> parent, so "the style said nothing" has to stay
/// distinguishable from "the style said the default". A toggle cannot express that on its own —
/// <c>\b0</c> and an absent <c>\b</c> both leave the group state not-bold — so which control words
/// the entry actually stated is recorded while it is read and consulted here.
/// </para>
/// </remarks>
public sealed record RtfStyleFormatting
{
    /// <summary>The <c>\f</c> index into the font table.</summary>
    public int? FontIndex { get; init; }

    /// <summary>The <c>\fs</c> size in half-points.</summary>
    public int? FontSizeHalfPoints { get; init; }

    /// <summary><c>\b</c>.</summary>
    public bool? Bold { get; init; }

    /// <summary><c>\i</c>.</summary>
    public bool? Italic { get; init; }

    /// <summary><c>\ul</c> and its siblings.</summary>
    public TextUnderline? Underline { get; init; }

    /// <summary><c>\strike</c>.</summary>
    public bool? Strike { get; init; }

    /// <summary><c>\caps</c>.</summary>
    public bool? Capitals { get; init; }

    /// <summary><c>\scaps</c>.</summary>
    public bool? SmallCapitals { get; init; }

    /// <summary><c>\cf</c>, as an index into the colour table.</summary>
    public int? ForegroundColourIndex { get; init; }

    /// <summary><c>\lang</c>.</summary>
    public int? LanguageId { get; init; }

    /// <summary>
    /// <c>\charscalex</c>, the character width as a percentage.
    /// </summary>
    /// <remarks>
    /// [bin] A paragraph style's and a character style's <c>\charscalex</c> each scale the run at
    /// 26.2.4.2 — <c>{\stylesheet{\s1\charscalex60 …}}</c> applied with <c>\s1</c> and
    /// <c>{\stylesheet{\*\cs2\charscalex60 …}}</c> applied with <c>\cs2</c> both draw at
    /// <b>0.59974</b> — so the stylesheet half is not optional: <b>53 of the converted column's 1520
    /// non-identity occurrences are declared in <c>{\stylesheet}</c> and nowhere else</b>, and the
    /// <c>ListLabel</c> character styles are exactly that shape.
    /// <c>probes/charscale-r149/results.md</c> §D.
    /// </remarks>
    public int? WidthPerCent { get; init; }

    /// <summary>The alignment one of the <c>\q</c> words gave, by page side.</summary>
    /// <remarks>
    /// Carried because <c>LN_CT_PPrBase_jc</c> has no entry in <c>getDefaultSPRM</c>, so nothing
    /// overrides it on the way to the paragraph. Reaches the text from an ancestor only.
    /// </remarks>
    public TextAlignment? Alignment { get; init; }

    /// <summary>The space above the paragraph in twips, from <c>\sb</c>.</summary>
    /// <remarks>
    /// Its sibling <c>\sa</c> is deliberately absent: <c>getDefaultSPRM</c>'s value for the whole
    /// <c>spacing</c> node is <c>after = 0</c>, which is written over any style's space after and
    /// leaves its space before alone. Reaches the text from an ancestor only.
    /// </remarks>
    public int? SpaceBeforeTwips { get; init; }

    /// <summary>The space below the paragraph in twips.</summary>
    /// <remarks>
    /// No RTF <c>\sa</c> ever reaches this — <c>getDefaultSPRM</c>'s value for the whole
    /// <c>spacing</c> node is <c>after = 0</c>, which is written over any style's space after.
    /// It carries one thing only: the space below that Writer's own <em>Heading</em> or
    /// <em>Caption</em> pool style gives a style whose <c>\sbasedon</c> did not resolve. That is
    /// not an RTF sprm at all, so nothing in <c>cloneAndDeduplicateSprm</c>'s table can
    /// overwrite it — and it also means a document's own <c>caption</c> entry's <c>\sa</c> cannot
    /// leak into a <c>COLL_LABEL_*</c> paragraph through this member, because no <c>\sa</c> is
    /// ever read into it. See
    /// <see cref="RtfStyles.PoolParentOf"/>.
    /// </remarks>
    public int? SpaceAfterTwips { get; init; }

    /// <summary><c>\keepn</c>.</summary>
    /// <remarks>
    /// The one paragraph property a style states <em>for itself</em> and still reaches the text,
    /// because <c>keepNext</c> is a plain value with no default and no children, so
    /// <c>cloneAndDeduplicateSprm</c>'s not-found branch writes nothing over it.
    /// </remarks>
    public bool? KeepWithNext { get; init; }

    /// <summary>True when the entry stated nothing this type carries.</summary>
    public bool IsEmpty => this == Empty;

    /// <summary>A style that states nothing.</summary>
    public static RtfStyleFormatting Empty { get; } = new();

    /// <summary>
    /// This style's formatting laid over <paramref name="parent"/>'s: what the child states wins,
    /// and what it leaves unstated is inherited.
    /// </summary>
    public RtfStyleFormatting Over(RtfStyleFormatting parent) => new()
    {
        FontIndex = FontIndex ?? parent.FontIndex,
        FontSizeHalfPoints = FontSizeHalfPoints ?? parent.FontSizeHalfPoints,
        Bold = Bold ?? parent.Bold,
        Italic = Italic ?? parent.Italic,
        Underline = Underline ?? parent.Underline,
        Strike = Strike ?? parent.Strike,
        Capitals = Capitals ?? parent.Capitals,
        SmallCapitals = SmallCapitals ?? parent.SmallCapitals,
        ForegroundColourIndex = ForegroundColourIndex ?? parent.ForegroundColourIndex,
        LanguageId = LanguageId ?? parent.LanguageId,
        WidthPerCent = WidthPerCent ?? parent.WidthPerCent,
        Alignment = Alignment ?? parent.Alignment,
        SpaceBeforeTwips = SpaceBeforeTwips ?? parent.SpaceBeforeTwips,
        SpaceAfterTwips = SpaceAfterTwips ?? parent.SpaceAfterTwips,
        KeepWithNext = KeepWithNext ?? parent.KeepWithNext,
    };
}

/// <summary>One entry from an RTF <c>{\stylesheet}</c> group.</summary>
/// <param name="Name">The style's name, as the trailing text of its definition.</param>
/// <param name="BasedOn">The style this one is based on, or null.</param>
/// <param name="OutlineLevel">
/// The zero-based outline level the style gives its paragraphs, or null for body text.
/// </param>
/// <param name="IsCharacterStyle">True for a <c>\cs</c> character style.</param>
/// <param name="Formatting">The formatting the entry states, relative to its parent.</param>
public readonly record struct RtfStyle(
    string Name,
    int? BasedOn,
    int? OutlineLevel,
    bool IsCharacterStyle,
    RtfStyleFormatting Formatting);

/// <summary>
/// What Writer's style pool gives an RTF style whose <c>\sbasedon</c> did not resolve.
/// </summary>
/// <remarks>
/// <para>
/// The answers are not the same kind of thing, which is why this is an enumeration rather than a
/// nullable formatting. <see cref="Standard"/> is a *reference*: what the paragraph inherits is
/// whatever the document's own <c>Normal</c> entry says, which is why a probe that measures one
/// <c>Normal</c> size cannot tell inheritance from a constant. <see cref="Heading"/> and
/// <see cref="Caption"/> are an intermediate pool style's own properties laid <em>over</em> that
/// reference, because every Writer paragraph pool style has <c>COLL_STANDARD</c> at the root of
/// its <c>GetPoolParent</c> chain (<c>sw/source/core/doc/poolfmt.cxx</c>:169-298) — and
/// <see cref="Caption"/> is a reference again whenever the document declares a
/// <c>caption</c> entry of its own. See <see cref="RtfStyles.PoolParentOf"/>.
/// </para>
/// </remarks>
internal enum RtfPoolParent
{
    /// <summary>The name is not one Writer already has a style for.</summary>
    None,

    /// <summary>Writer's <em>Heading</em>: the nine headings, <c>Title</c> and <c>Subtitle</c>.</summary>
    Heading,

    /// <summary>Writer's <em>Standard</em>, which is the document's own <c>Normal</c> entry.</summary>
    Standard,

    /// <summary>
    /// Writer's <em>Caption</em>, which is the five <c>COLL_LABEL_*</c> names' parent.
    /// </summary>
    Caption,
}

/// <summary>
/// The styles an RTF document declares, and the formatting they carry.
/// </summary>
/// <remarks>
/// <para>
/// It is often said that RTF's stylesheet is decorative — that a writer must restate a
/// paragraph's <em>effective</em> formatting inline after the <c>\s</c> that names its style, so a
/// reader has no cascade to resolve. That is what this class used to say, and it is false for the
/// files this project actually reads.
/// </para>
/// <para>
/// LibreOffice's own RTF export writes the <em>difference</em> from the style and nothing more, so
/// a paragraph in a style that already names the right font names no font at all:
/// <c>\pard\plain \s24\li85\lin85\intbl{\cf8\fs20\b TC Holder}</c> is a whole table cell whose
/// face is stated only by <c>\s24</c>, through <c>\sbasedon0</c>, in the stylesheet. Its importer
/// resolves that cascade — <c>RTFDocumentImpl::getProperties</c>
/// (<c>sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx</c>:534-637) merges the style's
/// properties under the paragraph's own, with the comment <em>"Take paragraph style into account
/// for character properties as well, as paragraph style may contain character properties"</em> at
/// :616-618 — and <c>\pard</c> selects style 0 when the paragraph names none
/// (<c>rtfdispatchflag.cxx</c>:600-614, <em>"By default the style with index 0 is applied"</em>).
/// </para>
/// <para>
/// Reading it the other way put every such run in font 0, which is <c>Times New Roman</c> in every
/// file LibreOffice writes: on <c>Annex-10…GCAA.rtf</c> the reference lays the whole 148-page table
/// out in Carlito and this tree laid it out in Liberation Serif, which is wide enough to wrap
/// cells the reference fits, and the document paginated to 169 pages.
/// </para>
/// </remarks>
public sealed class RtfStyles
{
    /// <summary>How deep a <c>\sbasedon</c> chain is followed.</summary>
    /// <remarks>A guard on untrusted input; a real chain is two or three deep.</remarks>
    private const int MaxBasedOnDepth = 32;

    private readonly Dictionary<int, RtfStyle> _paragraphStyles = [];
    private readonly Dictionary<int, RtfStyle> _characterStyles = [];
    private readonly Dictionary<int, RtfStyleFormatting> _resolved = [];
    private readonly Dictionary<int, RtfStyleFormatting> _resolvedContribution = [];

    /// <summary>
    /// The <c>\s</c> id of the entry that redefines Writer's <em>Caption</em>, if the document
    /// declares one.
    /// </summary>
    /// <remarks>
    /// The last such entry wins, because <c>ApplyStyleSheets</c> walks the table in order and each
    /// one resets the pool style and writes its own properties back over it
    /// (<c>StyleSheetTable.cxx</c>:1101-1121). A <c>caption</c> entry counts whether or not any
    /// paragraph applies it — it is the *declaration* that resets the pool style.
    /// </remarks>
    private int? _captionStyleId;

    /// <summary>Records a style definition read from the stylesheet.</summary>
    /// <remarks>
    /// A <c>\sbasedon</c> naming a style the sheet has not reached yet is dropped, because the
    /// importer resolves it to a <em>name</em> where it stands —
    /// <c>pIntValue = new RTFValue(getStyleName(nParam))</c>,
    /// <c>sw/source/writerfilter/rtftok/rtfdispatchvalue.cxx</c>:131-134 — and a style not yet read
    /// has no name to give, so <c>StyleSheetTable</c> never calls <c>setParentStyle</c> for it.
    /// Measured: a child declared before its parent takes nothing from it
    /// (<c>probes/rtf-resid-r80</c>, <c>o-childfirst</c>), where a child declared after it, with an
    /// unrelated entry in between, takes everything (<c>s-gap</c>).
    /// </remarks>
    internal void Add(int id, RtfStyle style)
    {
        if (style.BasedOn is { } parent && !_paragraphStyles.ContainsKey(parent))
        {
            style = style with { BasedOn = null };
        }

        if (style.IsCharacterStyle)
        {
            _characterStyles[id] = style;
        }
        else
        {
            _paragraphStyles[id] = style;
            string trimmed = style.Name.Trim();
            if (string.Equals(trimmed, "caption", StringComparison.Ordinal)
                || string.Equals(trimmed, "Caption", StringComparison.Ordinal))
            {
                _captionStyleId = id;
            }
        }

        _resolved.Clear();
        _resolvedContribution.Clear();
    }

    /// <summary>
    /// What Writer's own style pool gives a paragraph style whose <c>\sbasedon</c> did not
    /// resolve, or null when the name is not one the pool answers.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A style whose name is one of Word's is not created: Writer's existing style of that
    /// name is reused, and it keeps its own parent when the entry states no usable
    /// <c>\sbasedon</c>.</strong> <c>StyleSheetTable::ApplyStyleSheets</c>
    /// (<c>sw/source/writerfilter/dmapper/StyleSheetTable.cxx</c>:1099-1121) converts the entry's
    /// name through <c>ConvertStyleName</c> (<c>:1620-1660</c>, whose map holds
    /// <c>heading 1</c>…<c>heading 9</c> and <c>Heading 1</c>…<c>Heading 9</c>), and where
    /// <c>xStyles->hasByName</c> answers yes it takes that style, resets its own properties and
    /// leaves the parent alone — the one branch that would clear it needs
    /// <c>m_bHasImportedDefaultParaProps</c>, which only an OOXML <c>w:docDefaults</c> sets
    /// (<c>:653-667</c>), so for RTF it is never true. A resolvable <c>\sbasedon</c> replaces the
    /// parent instead (<c>:1156-1169</c>), which is why this is a fallback and not an override.
    /// </para>
    /// <para>
    /// <strong>Three families of name reach this, and only one of them is here.</strong>
    /// <c>heading 1</c>…<c>heading 9</c>, <c>Title</c> and <c>Subtitle</c> all map to a pool style
    /// under <c>COLL_DOC_BITS</c>, whose parent is <c>COLL_HEADLINE_BASE</c> unless it <em>is</em>
    /// that style (<c>GetPoolParent</c>, <c>sw/source/core/doc/poolfmt.cxx</c>:279-289) — so all
    /// eleven answer <em>Heading</em>, and none of them answers the 28 pt bold centring
    /// <c>COLL_DOC_TITLE</c> states for itself (<c>DocumentStylePoolManager.cxx</c>:1365-1374) or
    /// the 18 pt of <c>COLL_DOC_SUBTITLE</c> (<c>:1376-1387</c>), because the entry's own
    /// properties are what the import resets. Measured: <c>Title</c> and <c>Subtitle</c> reproduce
    /// <c>heading 4</c>'s size, space above and space below to the hundredth of a point.
    /// </para>
    /// <para>
    /// <c>Body Text</c> and <c>caption</c> are the second family and answer
    /// <see cref="RtfPoolParent.Standard"/>. Their pool styles' parent is <c>COLL_STANDARD</c>
    /// (<c>poolfmt.cxx</c>:201-204, :229-235), so what they inherit is the document's own
    /// <c>Normal</c> entry rather than a constant: the same probe with
    /// <c>{\s0 … \fs20 Normal;}</c> draws them at 10 pt and with <c>\fs28</c> at 14, while
    /// <c>heading 4</c>, <c>Title</c> and <c>Subtitle</c> answer 14 to both. Neither
    /// <c>COLL_TEXT</c>'s 0/7 pt nor <c>COLL_LABEL</c>'s italic 6/6 survives the reset, so the
    /// pool entry a reader would be tempted to copy is the wrong half of it — what carries is the
    /// <em>parent</em>, and modelling it is following the chain into style 0 rather than folding
    /// a constant in. <c>Caption</c> is the map's other spelling of the same style
    /// (<c>StyleSheetTable.cxx</c>:1715-1716).
    /// </para>
    /// <para>
    /// <b>Two routes reach a Writer style, not one, and the second is a set nobody had
    /// enumerated.</b> For a name the map does not hold, <c>ConvertStyleName</c> returns it
    /// <em>unchanged</em> (<c>:2083-2113</c>) — unless it collides with one of the map's own
    /// values, when it gets a <c> (WW)</c> suffix and then matches nothing — so the second lookup
    /// is on the name as the file wrote it. <c>SwXStyleFamily::hasByName</c>
    /// (<c>sw/source/core/unocore/unostyle.cxx</c>:1030-1039) is <c>FillUIName</c> followed by
    /// <c>Find</c>, and what it accepts is <b>the <em>programmatic</em> names of Writer's paragraph
    /// pool styles and not their UI names</b>: a name that is only a UI name takes
    /// <c>FillUIName</c>'s middle arm and is handed to <c>Find</c> with <c>" (user)"</c> appended
    /// (<c>sw/source/core/doc/SwStyleNameMapper.cxx</c>:306-335, the suffix at <c>:327</c>), so it
    /// matches nothing. A pool style that has not been created yet is still found, because
    /// <c>FillStyleSheet</c>'s <c>FillOnlyName</c> arm falls back to <c>GetPoolIdFromUIName</c>
    /// (<c>sw/source/uibase/app/docstyle.cxx</c>:2389-2416).
    /// <c>probes/rtf-style-r97/hasbyname-census.py</c> is that census over the 338 converted
    /// <c>.rtf</c>: of the 84 names they apply without a resolvable <c>\sbasedon</c>, <b>17 reach a
    /// Writer style</b> and three of the seventeen — <c>Figure</c>, <c>Heading</c> and
    /// <c>Text</c>, one document each — do it by this route rather than through the map. All three
    /// spell their pool style the same way in both tables, so the strict rule and the lenient one
    /// that was tried first give the same 17; the census prints the difference rather than
    /// assuming it is empty.
    /// </para>
    /// <para>
    /// <b>Every Writer paragraph pool style has <c>COLL_STANDARD</c> at the root of its chain</b>
    /// (<c>GetPoolParent</c>, <c>poolfmt.cxx</c>:169-298, transcribed group by group in that
    /// census), so the question is never <em>whether</em> the walk reaches <em>Standard</em> but
    /// what the intermediates add on the way. Measured at 26.2.4.2 on 116 one-name probes read out
    /// of the flat ODF (<c>probes/rtf-style-r97/genpool2.py</c>, <c>genpool3.py</c>):
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// <c>COLL_HEADERFOOTER</c> (<c>header</c>, <c>footer</c>) and <c>COLL_REGISTER_BASE</c>
    /// (<c>toc 1</c>…<c>toc 9</c>, <c>Index 1</c>…<c>Index 3</c>) add <b>nothing</b>. The two tab
    /// stops <c>DocumentStylePoolManager.cxx</c>:913-940 gives <em>Header and Footer</em> are in a
    /// native ODF import and are <em>absent</em> after an RTF one — both measured, and the second
    /// is why this is a measurement rather than a reading of that block.
    /// </description></item>
    /// <item><description>
    /// A bare <c>Heading</c> adds nothing either, and that is the trap in its name: it
    /// <em>is</em> <c>COLL_HEADLINE_BASE</c>, so the very style that supplies the heading
    /// constants is the one the import resets. It answers <see cref="RtfPoolParent.Standard"/>,
    /// not <see cref="RtfPoolParent.Heading"/>.
    /// </description></item>
    /// <item><description>
    /// <c>COLL_LABEL</c> — Writer's <em>Caption</em>, the parent of all five
    /// <c>COLL_LABEL_*</c> names <c>Figure</c>, <c>Illustration</c>, <c>Table</c>, <c>Drawing</c>
    /// and <c>Text</c> (<c>poolfmt.cxx</c>:247-252) — adds <b>italic, 12 pt and 6 pt above and
    /// below</b>. 12 pt rather than the <c>PT_10</c> of
    /// <c>DocumentStylePoolManager.cxx</c>:978, because <c>SwDocShell::InitNew</c>
    /// (<c>sw/source/uibase/app/docshini.cxx</c>:224-289) overwrites <c>COLL_LABEL</c>'s size with
    /// the configured <c>FONTSIZE_DEFAULT</c> of 240 twips in every new document — the same loop
    /// that gives <c>COLL_HEADLINE_BASE</c> its <c>FONTSIZE_OUTLINE</c> of 280.
    /// </description></item>
    /// <item><description>
    /// <c>Comment</c> and <c>Signature</c> are under <c>COLL_STANDARD</c> directly and add nothing.
    /// They are here because they are right and free: the corpus applies neither.
    /// </description></item>
    /// </list>
    /// <para>
    /// <b>What separates the two answers is when the pool style was created, not which one it
    /// is.</b> <c>DomainMapper</c> sets <c>StylesNoDefault</c> at the start of every writerfilter
    /// import — <em>"Don't load the default style definitions to avoid weird mix"</em>,
    /// <c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:141, put back to false at
    /// <c>:259</c> — and that flag is <c>bNoDefault</c> in
    /// <c>DocumentStylePoolManager::GetTextCollFromPool</c>
    /// (<c>DocumentStylePoolManager.cxx</c>:676-677), which skips the whole per-style
    /// <c>switch</c> while leaving the <c>GetPoolParent</c> linkage above it (<c>:660-674</c>)
    /// intact. So a pool style created <em>during</em> the import gets a parent and no properties
    /// — <c>COLL_HEADERFOOTER</c>, <c>COLL_HEADER</c>, <c>COLL_FOOTER</c>, <c>COLL_COMMENT</c>,
    /// <c>COLL_SIGNATURE</c> — and the four <c>SwDocShell::InitNew</c> creates before the filter
    /// runs keep everything. <c>COLL_REGISTER_BASE</c> is one of those four and is still
    /// transparent, because the size <c>InitNew</c> wants for it is the 240 twips it already
    /// inherits and the loop writes only a difference (<c>docshini.cxx</c>:279-288); measured, not
    /// derived — <c>p_index1_plain_20</c> reads <c>10pt@Standard</c> and <c>_28</c> reads
    /// <c>14pt@Standard</c>.
    /// </para>
    /// <para>
    /// <b>And <em>Caption</em> is a reference rather than a constant whenever the document
    /// declares a <c>caption</c> of its own</b>, because <c>SetPropertiesToDefault</c> resets it
    /// and the entry's own properties are written back over it. Both corpus documents that apply
    /// a <c>COLL_LABEL_*</c> name do exactly that, so the italic never arrives and what the
    /// paragraph takes is their <c>caption</c> entry's bold 10 pt.
    /// <c>probes/rtf-style-r97/genpool3.py</c> measures both arms, fourteen names × two
    /// <c>Normal</c> sizes.
    /// </para>
    /// <para>
    /// The control family is every name the map answers <em>nothing</em> for — <c>Quote</c>,
    /// <c>Normal (Web)</c> and <c>List Paragraph</c> among them (<c>:1794</c>, <c>:1883-1884</c>)
    /// — which keeps <c>\pard\plain</c>'s twelve points and is what this rule must not move.
    /// <c>Marginalia</c> and <c>Text body indent</c> are the same control by the other rule: they
    /// are Writer names, but they are also <em>values</em> in the map, so the <c> (WW)</c> suffix
    /// takes them out of reach.
    /// </para>
    /// <para>
    /// Every heading maps to a pool style whose parent is <em>Heading</em>, and
    /// <c>SwPoolFormatId::COLL_HEADLINE_BASE</c>
    /// (<c>sw/source/core/doc/DocumentStylePoolManager.cxx</c>:769-820) is where the four values
    /// below come from: <c>SvxFontHeightItem aFntSize(PT_14, …)</c> at <c>:809</c>,
    /// <c>SvxULSpaceItem aUL(PT_12, PT_6, …)</c> at <c>:810</c> and
    /// <c>SvxFormatKeepItem(true, RES_KEEP)</c> at <c>:813</c>. The per-level percentages in
    /// <c>aHeadlineSizes</c> (<c>:107-114</c>) do <em>not</em> arrive with them: the import resets
    /// the level style's own properties, so <c>heading 1</c> through <c>heading 9</c> all answer
    /// 14 pt. Measured on the reference, nine levels and four properties —
    /// <c>probes/rtf-holdover-r87/</c>.
    /// </para>
    /// <para>
    /// <b>And <em>Heading</em> is an intermediate rather than the end of the walk.</b>
    /// <c>COLL_HEADLINE_BASE</c>'s own pool parent is <c>COLL_STANDARD</c>
    /// (<c>sw/source/core/doc/poolfmt.cxx</c>:279-289, the <c>COLL_DOC_BITS</c> arm, where
    /// <c>COLL_HEADLINE1</c>…<c>COLL_HEADLINE10</c>, <c>COLL_DOC_TITLE</c> and
    /// <c>COLL_DOC_SUBTITLE</c> take the <c>default:</c> branch to <c>COLL_HEADLINE_BASE</c> and
    /// <c>COLL_HEADLINE_BASE</c> itself takes the one above it to <c>COLL_STANDARD</c>), so a
    /// document's own <c>Normal</c> reaches a heading in every property the intermediate leaves
    /// unstated. Rounds 87 and 95 both folded <see cref="RtfPoolParent.Heading"/> in as a constant
    /// and neither measured it; <c>probes/rtf-heading-r98/</c> does, 75 probes over twelve names
    /// and three controls, and the reference inherits <b>bold, italic, underline, strike,
    /// capitals, colour and alignment</b> from <c>Normal</c> at all twelve while shadowing size,
    /// space above, space below and keep with the intermediate's own. It is <em>not</em> the
    /// structural claim on its own that settles this: the intermediate states four things and
    /// every one of them shadows <c>Normal</c>, so which properties survive had to be measured
    /// property by property.
    /// </para>
    /// </remarks>
    /// <returns>Which pool style the name inherits from, or <see cref="RtfPoolParent.None"/>.</returns>
    internal static RtfPoolParent PoolParentOf(string name)
    {
        // Word strips whitespace around style names before the name is looked up
        // (rtfdocumentimpl.cxx:1594), and ConvertStyleName's map is case-sensitive with an entry
        // for each of the two spellings a file actually uses.
        string trimmed = name.Trim();

        if (CaptionParented.Contains(trimmed)) return RtfPoolParent.Caption;
        if (StandardParented.Contains(trimmed)) return RtfPoolParent.Standard;

        // `Title` and `Subtitle` have one spelling each in the map; the nine headings have two.
        if (string.Equals(trimmed, "Title", StringComparison.Ordinal)
            || string.Equals(trimmed, "Subtitle", StringComparison.Ordinal))
        {
            return RtfPoolParent.Heading;
        }

        if (IsNumbered(trimmed, "heading ", "Heading ")) return RtfPoolParent.Heading;
        if (IsNumbered(trimmed, "toc ", "TOC ")) return RtfPoolParent.Standard;
        if (IsNumbered(trimmed, "index ", "Index ") && trimmed[^1] <= '3')
        {
            return RtfPoolParent.Standard;
        }

        return RtfPoolParent.None;
    }

    /// <summary>
    /// <paramref name="name"/> is one of two exact spellings followed by one digit 1-9.
    /// </summary>
    /// <param name="name">The style's name, already trimmed.</param>
    /// <param name="lower">The map's lower-case spelling of the family's prefix.</param>
    /// <param name="upper">The map's other spelling of it.</param>
    /// <remarks>
    /// <b>Exactly two spellings, compared ordinally, because that is how many the map holds and
    /// the second lookup does not rescue a third.</b> <c>heading 1</c> and <c>Heading 1</c> are
    /// both entries (<c>StyleSheetTable.cxx</c>:1641-1658), and so are <c>index 1</c>/<c>Index 1</c>
    /// (<c>:1659-1676</c>, and only 1-3 of the nine have a Writer name) and <c>TOC 1</c>/<c>toc 1</c>
    /// (<c>:1677-1694</c>) — but nothing maps <c>HEADING 1</c>, and <c>hasByName</c> misses it too,
    /// because <c>SwStyleNameMapper::FillUIName</c> finds it in neither the programmatic nor the UI
    /// table and hands <c>Find</c> the name unchanged
    /// (<c>sw/source/core/doc/SwStyleNameMapper.cxx</c>:306-335). A case-insensitive comparison of
    /// the whole prefix would answer <em>Standard</em> for a name Writer has no pool style for.
    /// </remarks>
    private static bool IsNumbered(string name, string lower, string upper)
    {
        if (name.Length != lower.Length + 1) return false;
        if (name[^1] is < '1' or > '9') return false;
        ReadOnlySpan<char> prefix = name.AsSpan(0, lower.Length);
        return prefix.SequenceEqual(lower) || prefix.SequenceEqual(upper);
    }

    /// <summary>
    /// The names whose Writer style has <c>COLL_STANDARD</c> for a pool parent with nothing
    /// measurable in between.
    /// </summary>
    /// <remarks>
    /// <c>Body Text</c> is <c>Text body</c>/<c>COLL_TEXT</c> and both spellings of <c>caption</c>
    /// are <c>Caption</c>/<c>COLL_LABEL</c> (<c>StyleSheetTable.cxx</c>:1715-1716, :1761), each
    /// with <c>COLL_STANDARD</c> directly above. <c>header</c> and <c>footer</c> reach it through
    /// <c>COLL_HEADERFOOTER</c> and a bare <c>Heading</c> is <c>COLL_HEADLINE_BASE</c> itself;
    /// both intermediates are measured to add nothing. <c>Comment</c> and <c>Signature</c> are
    /// the two remaining <c>hasByName</c> names measured in <c>genpool3.py</c>, and are in
    /// because they are right and free — the corpus applies neither.
    /// </remarks>
    private static readonly HashSet<string> StandardParented = new(StringComparer.Ordinal)
    {
        "Body Text", "caption", "Caption",
        "header", "Header", "footer", "Footer",
        "Heading", "Comment", "Signature",
    };

    /// <summary>The five <c>COLL_LABEL_*</c> names, whose pool parent is Writer's Caption.</summary>
    /// <remarks><c>poolfmt.cxx</c>:247-252; all five measured in <c>genpool3.py</c>.</remarks>
    private static readonly HashSet<string> CaptionParented = new(StringComparer.Ordinal)
    {
        "Figure", "Illustration", "Table", "Drawing", "Text",
    };

    /// <summary>
    /// What a style whose <c>\sbasedon</c> did not resolve inherits from its pool parent.
    /// </summary>
    /// <param name="name">The style's own name, as the stylesheet entry states it.</param>
    /// <param name="id">Its own <c>\s</c> id, so that <c>Normal</c> cannot inherit from itself.</param>
    private RtfStyleFormatting PoolInheritance(string name, int id) => PoolParentOf(name) switch
    {
        RtfPoolParent.Heading => HeadingPool.Over(NormalInheritance(id)),
        RtfPoolParent.Standard => NormalInheritance(id),
        RtfPoolParent.Caption => _captionStyleId is { } caption && caption != id
            ? FormattingOf(caption)
            : CaptionPool.Over(NormalInheritance(id)),
        _ => RtfStyleFormatting.Empty,
    };

    /// <summary>The document's own <c>Normal</c>, or nothing when <em>this</em> is it.</summary>
    private RtfStyleFormatting NormalInheritance(int id)
        => id != DefaultStyleId ? FormattingOf(DefaultStyleId) : RtfStyleFormatting.Empty;

    /// <summary>
    /// The <c>\s</c> id of the document's own default paragraph style, which RTF fixes at zero.
    /// </summary>
    /// <remarks>
    /// <c>rtfdispatchflag.cxx</c>:600-614 — <em>"By default the style with index 0 is applied"</em>
    /// — so <c>Standard</c> is this entry and not a constant.
    /// </remarks>
    private const int DefaultStyleId = 0;

    /// <summary>Writer's <em>Heading</em> pool style, as an RTF style's inherited half.</summary>
    /// <remarks>
    /// <b>These four values and nothing else, because they are what the intermediate states and
    /// everything it leaves unstated comes from the document's own <c>Normal</c> instead.</b> The
    /// reference's own resolved <em>Heading</em> — read out of a <c>--convert-to fodt</c> of
    /// <c>probes/rtf-heading-r98/</c>'s probes, so it is 26.2.4.2's answer and not this tree's
    /// reading of a different version's source — is exactly
    /// <c>style:parent-style-name="Standard"</c> with a font, <c>fo:font-size="14pt"</c>,
    /// <c>fo:margin-top="0.1665in"</c>, <c>fo:margin-bottom="0.0835in"</c> and
    /// <c>fo:keep-with-next="always"</c>. The face is the one member deliberately absent: no
    /// style's face reaches a run at all (<see cref="ContributionOf"/>), so modelling the
    /// intermediate's <c>LATIN_HEADING</c> font would change nothing and modelling it as an RTF
    /// <c>\f</c> index is not possible.
    /// </remarks>
    private static readonly RtfStyleFormatting HeadingPool = new()
    {
        FontSizeHalfPoints = 28,
        SpaceBeforeTwips = 240,
        SpaceAfterTwips = 120,
        KeepWithNext = true,
    };

    /// <summary>
    /// Writer's <em>Caption</em> pool style, for a document that does not declare one of its own.
    /// </summary>
    /// <remarks>
    /// Italic and 6 pt above and below are <c>DocumentStylePoolManager.cxx</c>:971-984; the size
    /// is <em>not</em> that block's <c>PT_10</c> but the 240 twips
    /// <c>SwDocShell::InitNew</c> writes over it (<c>docshini.cxx</c>:224-289). Measured 12 pt and
    /// italic at 26.2.4.2 on all five <c>COLL_LABEL_*</c> names, under both <c>Normal</c> sizes.
    /// </remarks>
    private static readonly RtfStyleFormatting CaptionPool = new()
    {
        FontSizeHalfPoints = 24,
        Italic = true,
        SpaceBeforeTwips = 120,
        SpaceAfterTwips = 120,
    };

    /// <summary>The paragraph style with this <c>\s</c> id, or null.</summary>
    public RtfStyle? ParagraphStyle(int id)
        => _paragraphStyles.TryGetValue(id, out RtfStyle style) ? style : null;

    /// <summary>The character style with this <c>\cs</c> id, or null.</summary>
    public RtfStyle? CharacterStyle(int id)
        => _characterStyles.TryGetValue(id, out RtfStyle style) ? style : null;

    /// <summary>
    /// The formatting a paragraph in this style carries, with every <c>\sbasedon</c> ancestor
    /// merged under it.
    /// </summary>
    /// <remarks>
    /// Cycle-guarded and depth-bounded: a <c>\sbasedon</c> loop is malformed but appears in files
    /// written by converters, and this walks a chain over untrusted input.
    /// </remarks>
    public RtfStyleFormatting FormattingOf(int id)
    {
        if (_resolved.TryGetValue(id, out RtfStyleFormatting? cached)) return cached;

        RtfStyleFormatting result = RtfStyleFormatting.Empty;
        HashSet<int> visited = [];
        int? current = id;
        List<RtfStyleFormatting> chain = [];

        while (current is { } styleId && visited.Add(styleId) && chain.Count < MaxBasedOnDepth)
        {
            if (!_paragraphStyles.TryGetValue(styleId, out RtfStyle style)) break;
            chain.Add(style.Formatting);
            current = style.BasedOn;

            // The style at the top of the chain named no parent this sheet could resolve, so
            // Writer's own style of that name keeps the pool parent it came with. None of the
            // three ends the walk: `Standard` *is* the document's own `Normal`, which is style 0,
            // and that is what makes `\fs20 Normal` and `\fs28 Normal` give different answers;
            // `Heading` and `Caption` are an intermediate pool style laid over that same `Normal`,
            // because `COLL_HEADLINE_BASE` and `COLL_LABEL` both answer `COLL_STANDARD` for a pool
            // parent. `Caption` stops being the pool style's own values -- and becomes the
            // document's `caption` entry -- as soon as the document declares one, because the
            // import resets the pool style it matched.
            switch (style.BasedOn is null ? PoolParentOf(style.Name) : RtfPoolParent.None)
            {
                case RtfPoolParent.Heading:
                    chain.Add(HeadingPool);
                    current = DefaultStyleId;
                    break;
                case RtfPoolParent.Standard:
                    current = DefaultStyleId;
                    break;
                case RtfPoolParent.Caption when _captionStyleId is { } caption
                                                && caption != styleId:
                    current = caption;
                    break;
                case RtfPoolParent.Caption:
                    chain.Add(CaptionPool);
                    current = DefaultStyleId;
                    break;
                default:
                    break;
            }
        }

        // Nearest ancestor last, so each generation lays its own statements over its parent's.
        for (int i = chain.Count - 1; i >= 0; i--) result = chain[i].Over(result);

        _resolved[id] = result;
        return result;
    }

    /// <summary>
    /// What a paragraph naming this style actually takes from it, which is not the whole
    /// <c>\sbasedon</c> merge <see cref="FormattingOf"/> answers.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three classes, and which property is in which is measured rather than chosen — see the
    /// table in <see cref="RtfStyleFormatting"/>'s own remarks. Most properties are honoured only
    /// when an <em>ancestor</em> states them, because the named style's own statement is written
    /// back over the paragraph as RTF's default. A few — the ones <c>getDefaultSPRM</c> answers
    /// nothing for — are honoured wherever in the chain they are stated. The font <em>face</em> is
    /// in neither class: no style's face ever reaches a run.
    /// </para>
    /// <para>
    /// A null member means "leave the paragraph where <c>\pard\plain</c> left it", which is the
    /// same value the reset would write, so the two cases need not be told apart downstream.
    /// </para>
    /// </remarks>
    public RtfStyleFormatting ContributionOf(int id)
    {
        if (_resolvedContribution.TryGetValue(id, out RtfStyleFormatting? cached)) return cached;

        bool known = _paragraphStyles.TryGetValue(id, out RtfStyle style);
        RtfStyleFormatting own = known ? style.Formatting : RtfStyleFormatting.Empty;
        RtfStyleFormatting inherited = known && style.BasedOn is { } parent && parent != id
            ? FormattingOf(parent)
            : known ? PoolInheritance(style.Name, id)
            : RtfStyleFormatting.Empty;
        RtfStyleFormatting chain = FormattingOf(id);

        // A property the *named* style states is dropped and the ancestors' value with it, because
        // the reset is written over the paragraph as direct formatting; a property only an ancestor
        // states is honoured. Null therefore means "leave the paragraph at what \pard\plain left",
        // which is the same value the reset writes.
        static T? Inherited<T>(T? own, T? inherited) where T : struct
            => own is null ? inherited : null;

        RtfStyleFormatting result = new()
        {
            // Not the face: no style's font reaches a run at all -- see the type's remarks.
            FontSizeHalfPoints = Inherited(own.FontSizeHalfPoints, inherited.FontSizeHalfPoints),
            Bold = Inherited(own.Bold, inherited.Bold),
            Italic = Inherited(own.Italic, inherited.Italic),
            Underline = Inherited(own.Underline, inherited.Underline),
            ForegroundColourIndex =
                Inherited(own.ForegroundColourIndex, inherited.ForegroundColourIndex),
            Alignment = Inherited(own.Alignment, inherited.Alignment),
            SpaceBeforeTwips = Inherited(own.SpaceBeforeTwips, inherited.SpaceBeforeTwips),

            // Only ever the pool's, and the paragraph's own `\sa` still overrides it.
            SpaceAfterTwips = inherited.SpaceAfterTwips,

            // The properties `getDefaultSPRM` answers nothing for: the named style's own statement
            // survives, so these resolve over the whole chain.
            Capitals = chain.Capitals,
            SmallCapitals = chain.SmallCapitals,
            Strike = chain.Strike,
            LanguageId = chain.LanguageId,
            WidthPerCent = chain.WidthPerCent,
            KeepWithNext = chain.KeepWithNext,
        };

        _resolvedContribution[id] = result;
        return result;
    }

    /// <summary>
    /// The outline level a paragraph style gives, following <c>\sbasedon</c> when the style
    /// itself does not say.
    /// </summary>
    /// <remarks>
    /// Cycle-guarded: a <c>\sbasedon</c> loop is malformed but appears in files written by
    /// converters, and this walks a chain over untrusted input.
    /// </remarks>
    public int? OutlineLevelOf(int id)
    {
        HashSet<int> visited = [];
        int? current = id;

        while (current is { } styleId && visited.Add(styleId))
        {
            if (_paragraphStyles.TryGetValue(styleId, out RtfStyle style))
            {
                if (style.OutlineLevel is { } level) return level;
                current = style.BasedOn;
            }
            else
            {
                return null;
            }
        }
        return null;
    }
}
