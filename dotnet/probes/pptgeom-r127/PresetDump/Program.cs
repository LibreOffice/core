// Dumps this tree's DrawingML preset outlines as SVG path data, one line per request:
//   <name>\t<stroked subpaths>\t<whole outline>
// Input lines are `<preset name>` or `<preset name>\t<adj1>,<adj2>,...`; the values are given to
// the preset's own adjustment guides in the order the preset declares them.
// A square box makes `ss = min(w,h)` equal to both edges, so the two vocabularies' view-box
// anisotropy cannot enter the comparison; a 2:1 box is what separates a guide measured across the
// width from one measured across `ss`.
using System.Globalization;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Presentations.Layout;

double width = args.Length > 1 ? double.Parse(args[1], CultureInfo.InvariantCulture) : 4000;
double height = args.Length > 2 ? double.Parse(args[2], CultureInfo.InvariantCulture) : width;
DocSize size = new(Length.FromMm100((long)width), Length.FromMm100((long)height));

foreach (string line in File.ReadAllLines(args[0]))
{
    if (line.Length == 0) continue;
    string[] fields = line.Split('\t');
    string name = fields[0];
    if (PresetShapeGeometry.Find(name) is not { } definition) { Console.WriteLine($"{name}\tUNKNOWN\t"); continue; }

    Dictionary<string, double>? adjustments = null;
    if (fields.Length > 1 && fields[1].Length > 0)
    {
        adjustments = [];
        string[] values = fields[1].Split(',');
        for (int i = 0; i < values.Length && i < definition.Adjustments.Count; i++)
            adjustments[definition.Adjustments[i].Name] = double.Parse(values[i], CultureInfo.InvariantCulture);
    }

    CustomShapeGeometry.Geometry geometry = SlidePresetGeometry.Of(name, size, adjustments);
    Console.WriteLine($"{name}\t{Describe(geometry.StrokeOutline)}\t{Describe(geometry.Outline)}");
}

static string Describe(GraphicsPath path)
{
    System.Text.StringBuilder text = new();
    foreach (PathCommand command in path.Commands)
    {
        switch (command.Verb)
        {
            case PathVerb.MoveTo: text.Append(CultureInfo.InvariantCulture, $"M {F(command.Point)} "); break;
            case PathVerb.LineTo: text.Append(CultureInfo.InvariantCulture, $"L {F(command.Point)} "); break;
            case PathVerb.CubicTo:
                text.Append(
                    CultureInfo.InvariantCulture,
                    $"C {F(command.Control1)} {F(command.Control2)} {F(command.Point)} ");
                break;
            case PathVerb.Close: text.Append("Z "); break;
            default: break;
        }
    }

    return text.ToString();
}

static string F(DocPoint point)
    => string.Create(CultureInfo.InvariantCulture, $"{point.X.Mm100},{point.Y.Mm100}");
