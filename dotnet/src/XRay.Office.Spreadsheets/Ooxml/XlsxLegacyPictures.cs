using System.Xml.Linq;
using XRay.Office.Containers;
using XRay.Office.Containers.Ooxml;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Vector;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The pictures on a worksheet's <em>legacy</em> drawing — the VML part its
/// <c>&lt;legacyDrawing&gt;</c> names — which is where a camera-tool picture and an OLE object's
/// preview actually live.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why a sheet needs this at all, when it already has a DrawingML drawing part.</strong>
/// Excel writes a camera-tool picture twice: as an <c>xdr:pic</c> inside
/// <c>mc:Choice Requires="a14"</c>, and as a <c>v:shape</c> in the legacy VML beside it. `oox` does
/// not honour <c>a14</c> — <c>ContextHandler2Helper::prepareMceContext</c> has it commented out
/// with "we do not currently support inline formulas and other a14 stuff" — so Calc reads *none*
/// of the DrawingML anchor and draws the VML shape. See
/// <c>OoxmlNamespaces.UnderstoodExtensions</c>, where the other half of this lives.
/// </para>
/// <para>
/// Measured on 26.2.4.2 with five authored variants of
/// <c>013_Contextures_chart_sample</c> (<c>probes/sheets-r55/probe-vml-camera.py</c>), each
/// changing one thing and each with a stated expected direction:
/// removing the sheet's <c>legacyDrawing</c> relationship makes the picture <strong>disappear</strong>
/// (23 extractable words to 5); moving the VML anchor's <c>to</c> column 6 → 9 <strong>widens</strong>
/// it; halving the VML <c>style</c> width changes <strong>nothing</strong>; deleting the
/// <c>x:Anchor</c> narrows it to the <c>style</c> rectangle. Round 54 had varied <c>editAs</c>,
/// <c>a:ext</c> and the DrawingML <c>to</c> column and found the reference inert to all three,
/// which is what a document whose DrawingML anchor is never read looks like.
/// </para>
/// <para>
/// <strong>The relationship <em>type</em> is the wrong key and would break three matching
/// documents.</strong> <c>legacyDrawingHF</c> — the header and footer's watermark images — uses the
/// same <c>vmlDrawing</c> relationship type, and keying on the type would draw
/// <c>PBN Matrix NAAs (V01)</c>'s 24 header images, <c>UAE Type Accepted Aircraft Models</c>'s and
/// <c>Application_Compliance_Checklist</c>'s one each as objects on the sheet. So the worksheet's
/// own <c>&lt;legacyDrawing r:id&gt;</c> is followed instead.
/// </para>
/// <para>
/// <strong>What is read, and what is knowingly not.</strong> <c>VmlDrawing::isShapeSupported</c>
/// (<c>sc/source/filter/oox/drawingfragment.cxx</c>) imports every VML shape whose
/// <c>x:ClientData</c> is absent or whose <c>ObjectType</c> is anything but <c>Note</c> — pictures,
/// OLE previews <em>and</em> the legacy form controls (Button, Checkbox, Drop, Scroll…), which it
/// rebuilds as OLE form controls. This reads the ones carrying <c>v:imagedata</c> and no others.
/// Censused over all 946 corpus documents (<c>probes/sheets-r55/census-vml.py</c>): 5 <c>Pict</c>
/// shapes in 3 spreadsheets, 1 <c>Scroll</c> — hidden, in <c>015_Free_Gantt_Chart_Template</c> — and
/// 359 <c>Note</c>s, which <see cref="XlsxNoteCaptions"/> already draws.
/// </para>
/// <para>
/// <strong>A shape with no <c>x:Anchor</c> is skipped, and that is a gap with a number on it.</strong>
/// <c>ShapeBase::calcShapeRectangle</c> (<c>oox/source/vml/vmlshape.cxx:509-516</c>) falls back to
/// the shape's CSS <c>position</c>/<c>width</c>/<c>height</c> when the client anchor is missing or
/// unusable, and the probe above confirms it on the binary: with <c>013</c>'s <c>x:Anchor</c>
/// deleted the reference draws the picture at the <c>style</c> rectangle, moving its first label
/// from x = 133.8 to 112.6. No corpus worksheet reaches that arm, so it is recorded rather than
/// written.
/// </para>
/// </remarks>
internal static class XlsxLegacyPictures
{
    private const string RelationshipNamespace =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>Reads the pictures on one sheet's legacy VML drawing.</summary>
    /// <param name="package">The workbook's package.</param>
    /// <param name="sheetPartName">The worksheet part the legacy drawing hangs off.</param>
    /// <param name="worksheet">
    /// The worksheet's own XML, for its <c>&lt;legacyDrawing&gt;</c> — the relationship type alone
    /// cannot tell that part from the header and footer's, see the remarks above.
    /// </param>
    public static List<SheetDrawing> Read(
        IPackage package, string? sheetPartName, XElement? worksheet)
    {
        ArgumentNullException.ThrowIfNull(package);

        List<SheetDrawing> pictures = [];
        if (sheetPartName is null || worksheet is null) return pictures;
        if (package is not OpcPackage opc) return pictures;

        string? id = worksheet
            .Element(XName.Get("legacyDrawing", OoxmlNamespaces.SpreadsheetML))
            ?.Attribute(XName.Get("id", RelationshipNamespace))?.Value;

        if (id is null) return pictures;

        OpcXml.Relationship? drawing = null;
        foreach (OpcXml.Relationship candidate in opc.GetRelationships(sheetPartName))
        {
            if (candidate.Id == id) { drawing = candidate; break; }
        }

        if (drawing is not { IsExternal: false } part) return pictures;
        if (opc.GetPart(part.Target) is not { } vml) return pictures;

        XElement? root;
        using (Stream content = vml.Open())
        {
            // A VML part is not namespace-well-formed by XML's rules on every producer, so a
            // failed load is ordinary and silent, as it is for every other optional part.
            root = OoxmlXml.TryLoad(content, out _);
        }

        if (root is null) return pictures;

        // A picture's `o:relid` resolves against the *VML part*, never against the sheet — the
        // same rule the DrawingML drawing follows, and the same mistake it is easy to make.
        Dictionary<string, OpcXml.Relationship> images = [];
        foreach (OpcXml.Relationship image in opc.GetRelationships(vml.Name))
            images[image.Id] = image;

        foreach (XElement shape in root.Descendants(XName.Get("shape", XlsxVml.Namespace)))
        {
            XElement? client = XlsxVml.ClientData(shape);
            if (XlsxVml.ObjectType(client) == "Note") continue;
            if (!XlsxVml.IsVisible(shape.Attribute("style")?.Value)) continue;

            XElement? data = shape.Element(XName.Get("imagedata", XlsxVml.Namespace));
            if (data is null) continue;

            // `o:relid` on nearly every producer; `r:id` is the transitional spelling and costs
            // one lookup to accept.
            string? relationship =
                data.Attribute(XName.Get("relid", XlsxVml.OfficeNamespace))?.Value
                ?? data.Attribute(XName.Get("id", RelationshipNamespace))?.Value;

            string? anchor = client?.Element(XName.Get("Anchor", XlsxVml.ExcelNamespace))?.Value;
            if (XlsxVml.ParseAnchor(anchor) is not { } placed) continue;

            (RasterImage? raster, Lazy<VectorImage>? vector) =
                XlsxDrawings.LoadImage(opc, images, relationship);

            if (raster is null && vector is null) continue;

            pictures.Add(new SheetDrawing
            {
                // `ANCHOR_VML` takes its size from the second corner exactly as `ANCHOR_TWOCELL`
                // does (`ShapeAnchor::calcAnchorRectEmu`, the two share a case label), so the
                // anchor kind is the two-cell one and no third kind is needed.
                Anchor = SheetAnchorKind.TwoCell,
                From = placed.From,
                To = placed.To,
                Name = shape.Attribute("id")?.Value,
                Image = vector is null ? raster : null,
                Vector = vector,
            });
        }

        return pictures;
    }
}
