using System.Collections.Frozen;
using System.Reflection;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// The 187 DrawingML preset shape definitions, read from the embedded table.
/// </summary>
/// <remarks>
/// <para>
/// The definitions are data and are kept as data: <c>PresetShapeGeometry.txt</c> is
/// LibreOffice's own <c>presetShapeDefinitions.xml</c> reduced to the four things drawing needs —
/// the adjustment defaults, the guides, the text rectangle and the paths — one line per element.
/// It is 110 kB, which is small enough to embed and far smaller than the 539 kB source, and it
/// stays diffable against that source, which a table of C# literals would not.
/// </para>
/// <para>
/// Parsed once, on first use, and held for the process. A deck asks for a handful of distinct
/// presets and a corpus run asks for a few dozen, so parsing per shape would repeat the same work
/// hundreds of times; parsing eagerly at startup would charge every caller who never draws a shape
/// at all, including every extraction.
/// </para>
/// </remarks>
public static class PresetShapeGeometry
{
    private static readonly Lazy<FrozenDictionary<string, PresetShape>> Shapes =
        new(Load, LazyThreadSafetyMode.ExecutionAndPublication);

    /// <summary>Every preset name this knows, in the order the table defines them.</summary>
    public static IReadOnlyCollection<string> Names => Shapes.Value.Keys;

    /// <summary>The definition of a preset, or null when the name is not one.</summary>
    /// <param name="name">The <c>a:prstGeom/@prst</c> value.</param>
    public static PresetShape? Find(string? name)
        => name is not null && Shapes.Value.TryGetValue(name, out PresetShape? shape) ? shape : null;

    private static FrozenDictionary<string, PresetShape> Load()
    {
        Dictionary<string, PresetShape> shapes = new(StringComparer.Ordinal);

        using Stream? stream = typeof(PresetShapeGeometry).Assembly
            .GetManifestResourceStream(
                $"{nameof(Paperless)}.{nameof(Ooxml)}.{nameof(DrawingML)}.PresetShapeGeometry.txt");

        if (stream is null) return shapes.ToFrozenDictionary(StringComparer.Ordinal);

        using StreamReader reader = new(stream);

        string? name = null;
        List<(string, string)> adjustments = [];
        List<(string, string)> guides = [];
        string[]? rectangle = null;
        List<PresetPath> paths = [];
        List<PresetCommand> commands = [];
        string pathWidth = "0";
        string pathHeight = "0";

        void FlushPath()
        {
            if (commands.Count > 0) paths.Add(new PresetPath(pathWidth, pathHeight, commands));
            commands = [];
        }

        void Flush()
        {
            FlushPath();
            if (name is not null)
                shapes[name] = new PresetShape(name, adjustments, guides, rectangle, paths);

            adjustments = [];
            guides = [];
            rectangle = null;
            paths = [];
        }

        while (reader.ReadLine() is { } line)
        {
            if (line.Length == 0 || line[0] == '#') continue;

            // Every line is a one-character kind, a space, then space-separated fields; a formula
            // is the rest of the line and keeps its own spaces, which is why the split is bounded.
            char kind = line[0];
            string rest = line.Length > 2 ? line[2..] : string.Empty;

            switch (kind)
            {
                case 's':
                    Flush();
                    name = rest;
                    break;

                case 'a':
                case 'g':
                {
                    int space = rest.IndexOf(' ', StringComparison.Ordinal);
                    if (space <= 0) break;

                    (string, string) guide = (rest[..space], rest[(space + 1)..]);
                    if (kind == 'a') adjustments.Add(guide);
                    else guides.Add(guide);
                    break;
                }

                case 'r':
                {
                    string[] fields = rest.Split(' ');
                    if (fields.Length == 4) rectangle = fields;
                    break;
                }

                case 'p':
                {
                    FlushPath();
                    string[] fields = rest.Split(' ');
                    pathWidth = fields.Length > 0 ? fields[0] : "0";
                    pathHeight = fields.Length > 1 ? fields[1] : "0";
                    break;
                }

                case 'm':
                    commands.Add(new PresetCommand(PresetVerb.MoveTo, rest.Split(' ')));
                    break;

                case 'l':
                    commands.Add(new PresetCommand(PresetVerb.LineTo, rest.Split(' ')));
                    break;

                case 'x':
                    commands.Add(new PresetCommand(PresetVerb.ArcTo, rest.Split(' ')));
                    break;

                case 'q':
                    commands.Add(new PresetCommand(PresetVerb.QuadraticTo, rest.Split(' ')));
                    break;

                case 'b':
                    commands.Add(new PresetCommand(PresetVerb.CubicTo, rest.Split(' ')));
                    break;

                case 'z':
                    commands.Add(new PresetCommand(PresetVerb.Close, []));
                    break;

                default:
                    break;
            }
        }

        Flush();
        return shapes.ToFrozenDictionary(StringComparer.Ordinal);
    }
}
