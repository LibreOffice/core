using System.Xml.Linq;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;

namespace XRay.Office.Presentations.OpenDocument;

/// <summary>
/// Which of a master page's four running objects — header, footer, date-time and slide number —
/// reach one slide, and what their fields say there.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A master's running objects are the one family of presentation object it draws on the
/// slides beneath it, and the switch is the slide's, not the master's.</strong>
/// <c>SdPage::CreatePresObj</c> marks a master's <em>title</em>
/// (<c>sd/source/core/sdpage.cxx</c>:305-310), <em>outline</em> (:314-321) and <em>notes</em>
/// (:325-332) placeholders <c>SetNotVisibleAsMaster(true)</c> and marks nothing else, and
/// <c>ViewObjectContactOfSdrObj::isPrimitiveVisible</c>
/// (<c>svx/source/sdr/contact/viewobjectcontactofsdrobj.cxx</c>:81-85) drops any object so
/// marked while a master is drawn as a slide's sub-content. A subtitle is
/// <c>PresObjKind::Text</c> (<c>sd/source/ui/unoidl/unopage.cxx</c>:427-430) and is dropped a
/// line later, by <c>SdPage::checkVisibility</c>'s <em>"presentation objects on master slide are
/// always invisible if slide is shown"</em> branch (<c>sdpage.cxx</c>:2986-2991).
/// </para>
/// <para>
/// The header, footer, date-time and slide-number placeholders take neither route. They are
/// answered by the branch <em>above</em> that one (<c>sdpage.cxx</c>:2957-2984), which reads the
/// <strong>visualised page's</strong> <c>HeaderFooterSettings</c> — so each is drawn on exactly
/// the slides whose own drawing-page style switches it on. ODF states those four bits as
/// <c>presentation:display-header</c>, <c>-footer</c>, <c>-date-time</c> and
/// <c>-page-number</c> (<c>xmloff/source/draw/sdpropls.cxx</c>:376-379).
/// </para>
/// <para>
/// <strong>The defaults are not all the same, and only one of the four is off.</strong>
/// <c>HeaderFooterSettings</c>'s constructor (<c>sdpage.cxx</c>:3222-3230) sets header, footer
/// and date-time visible and the <em>slide number</em> hidden. Every one of the converted
/// corpus's 8810 page-to-running-object pairings states the attribute explicitly, so the corpus
/// cannot witness a default; they are followed here because a hand-written file can.
/// </para>
/// <para>
/// <strong>What the object draws is its own text, not the declaration's — unless it holds a
/// field.</strong> LibreOffice's ODF export writes a master's running object out in one of two
/// shapes, and they behave differently: the frame either carries the characters directly, in
/// which case those characters are drawn on every slide the switch reaches, or it carries a
/// <c>presentation:footer</c> / <c>presentation:header</c> / <c>presentation:date-time</c>
/// element, which is a field and resolves against <em>the slide's</em>
/// <c>presentation:use-footer-name</c> and siblings through the document's
/// <c>presentation:footer-decl</c> declarations (<c>xmloff/source/draw/ximppage.cxx</c>:301-360
/// puts the declaration's text on the page as <c>FooterText</c>;
/// <c>sd/source/ui/app/sdmod2.cxx</c>:374-425 is what a <c>SvxFooterField</c> then reads).
/// Both shapes are common: over the 302 converted <c>.odp</c> the master frames divide
/// 983 footers carrying a field against 11 carrying characters, and 992 date-times against 18.
/// </para>
/// <para>
/// Measured on <c>ws_prod-…-M.017-(French)-France.odp</c>, whose master states both at once: its
/// footer declaration says <c>DGINT/2</c> and its master's footer frame holds the characters
/// <c>WG M.017: …</c>, and 26.2.4.2 draws the characters. On
/// <c>introduction_to_bea_tuxedo.odp</c>, whose master frames hold fields, it draws the
/// declarations.
/// </para>
/// </remarks>
internal sealed record OdpRunningObjects
{
    /// <summary>The four <c>presentation:class</c> values this covers.</summary>
    private const string HeaderClass = "header";
    private const string FooterClass = "footer";
    private const string DateTimeClass = "date-time";
    private const string PageNumberClass = "page-number";

    /// <summary>
    /// What a slide that has no running objects at all supplies: the slide number, so a
    /// <c>text:page-number</c> in the slide's own text still resolves, and nothing else.
    /// </summary>
    public static OdpRunningObjects ForPage(OdfStyles styles, XElement page, int index)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(page);

        string? style = page.Attribute(XName.Get("style-name", OdfNamespaces.Draw))?.Value;

        return new OdpRunningObjects
        {
            PageNumber = (index + 1).ToString(System.Globalization.CultureInfo.InvariantCulture),
            Header = Declared(page, "use-header-name", "header-decl"),
            Footer = Declared(page, "use-footer-name", "footer-decl"),
            DateTime = Declared(page, "use-date-time-name", "date-time-decl"),
            ShowsHeader = Switched(styles, style, "display-header", byDefault: true),
            ShowsFooter = Switched(styles, style, "display-footer", byDefault: true),
            ShowsDateTime = Switched(styles, style, "display-date-time", byDefault: true),
            ShowsPageNumber = Switched(styles, style, "display-page-number", byDefault: false),
        };
    }

    /// <summary>The slide's own number, one-based, as a <c>text:page-number</c> draws it.</summary>
    public required string PageNumber { get; init; }

    /// <summary>The text a <c>presentation:header</c> field draws, when the slide names one.</summary>
    public string? Header { get; init; }

    /// <summary>The text a <c>presentation:footer</c> field draws, when the slide names one.</summary>
    public string? Footer { get; init; }

    /// <summary>The text a <c>presentation:date-time</c> field draws, when the slide names one.</summary>
    public string? DateTime { get; init; }

    /// <summary>Whether the master's header reaches this slide.</summary>
    public bool ShowsHeader { get; init; }

    /// <summary>Whether the master's footer reaches this slide.</summary>
    public bool ShowsFooter { get; init; }

    /// <summary>Whether the master's date-time reaches this slide.</summary>
    public bool ShowsDateTime { get; init; }

    /// <summary>Whether the master's slide number reaches this slide.</summary>
    public bool ShowsPageNumber { get; init; }

    /// <summary>
    /// Whether a <c>presentation:class</c> frame on the master is drawn under this slide.
    /// </summary>
    /// <remarks>
    /// Only the four running kinds ever are, and each only when the slide switches it on. Every
    /// other class — a title, an outline, a subtitle, a notes body, a page thumbnail — is a
    /// placeholder the slide restates for itself and is never drawn from the master.
    /// </remarks>
    public bool Inherits(string presentationClass) => presentationClass switch
    {
        HeaderClass => ShowsHeader,
        FooterClass => ShowsFooter,
        DateTimeClass => ShowsDateTime,
        PageNumberClass => ShowsPageNumber,
        _ => false,
    };

    private static bool Switched(OdfStyles styles, string? style, string name, bool byDefault)
    {
        OdfProperty stated = styles.ResolveProperty(
            style,
            OdfStyleFamily.DrawingPage,
            OdfPropertyKind.DrawingPage,
            OdfNamespaces.Presentation,
            name);

        return stated.HasValue ? stated.Is("true") : byDefault;
    }

    /// <summary>
    /// The declaration a page names, from the document body's <c>presentation:*-decl</c> list.
    /// </summary>
    /// <remarks>
    /// The declarations are children of <c>office:presentation</c>, beside the pages, and are
    /// shared: several slides commonly name one. A page that names none has no text for that
    /// field, and a field with no text draws nothing.
    /// </remarks>
    private static string? Declared(XElement page, string use, string declaration)
    {
        if (page.Attribute(XName.Get(use, OdfNamespaces.Presentation))?.Value is not { } name)
        {
            return null;
        }

        XName element = XName.Get(declaration, OdfNamespaces.Presentation);
        XName named = XName.Get("name", OdfNamespaces.Presentation);

        foreach (XElement declared in page.Parent?.Elements(element) ?? [])
        {
            if (declared.Attribute(named)?.Value == name) return declared.Value;
        }

        return null;
    }
}
