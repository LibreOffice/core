using System.Xml.Linq;

namespace Paperless.Text.Fonts;

/// <summary>
/// The families a machine's fontconfig configuration prefers for a generic family.
/// </summary>
/// <remarks>
/// <para>
/// This exists for one question, asked at the very end of glyph fallback: <em>nothing on
/// LibreOffice's own fallback list is installed and several faces on this machine cover the
/// character — which one does LibreOffice draw it in?</em> The answer is not a property of the
/// fonts. It is a property of the machine's fontconfig configuration, and that was established by
/// measurement rather than assumed: with the machine's <c>/etc/fonts</c> in force,
/// <c>fc-match "宋体:charset=624b"</c> answers <em>WenQuanYi Zen Hei</em>; with a minimal
/// configuration naming only the font directory it answers <em>IPAGothic</em>, because nothing but
/// the scan order then separates the candidates. The difference is
/// <c>/etc/fonts/conf.d/64-wqy-zenhei.conf</c>, which lists WenQuanYi Zen Hei under
/// <c>&lt;alias&gt;&lt;family&gt;sans-serif&lt;/family&gt;&lt;prefer&gt;</c>.
/// </para>
/// <para>
/// So no rule derived from the font files — code-page bits, Unicode ranges, coverage size, name —
/// can reproduce the reference renderer's choice, and the resolver's previous tie-break
/// (alphabetical by family, which its own comment described as having no basis) put IPAGothic and
/// Unifont ahead of WenQuanYi Zen Hei on every Han character. Reading the configuration is
/// therefore not a second source of truth competing with LibreOffice's substitution table; it is
/// the source of truth for the one decision the table does not make. LibreOffice asks fontconfig
/// through <c>FcFontSetMatch</c> in <c>vcl/unx/generic/font/fontconfig.cxx</c> before it ever
/// consults its own list.
/// </para>
/// <para>
/// Deliberately narrow. Only <c>&lt;alias&gt;</c> elements whose subject is a *generic* family
/// (<c>serif</c>, <c>sans-serif</c>, <c>monospace</c> and friends) and whose body is a
/// <c>&lt;prefer&gt;</c> list are read. The metric-alias files — <c>Helvetica</c> preferring
/// <c>Nimbus Sans</c> — are aliases of a concrete family and say nothing about which face should
/// draw a character nobody's font covers, so they are skipped. Conditional
/// <c>&lt;match target="pattern"&gt;</c> rules are skipped too: they turn on a language or a size
/// this resolver does not carry, and guessing at their conditions would be worse than not
/// reproducing them.
/// </para>
/// <para>
/// The order is fontconfig's own. A <c>&lt;prefer&gt;</c> list is an
/// <c>&lt;edit name="family" mode="prepend"&gt;</c> applied at the position of the matched family,
/// so each file's entries land *behind* those of the files already processed and configuration
/// files rank in ascending name order. That is checkable on this machine:
/// <c>fc-match sans-serif</c> answers DejaVu Sans (from <c>57-dejavu-sans.conf</c>) and WenQuanYi
/// Zen Hei second (from <c>64-wqy-zenhei.conf</c>), which is the order the file names give.
/// </para>
/// </remarks>
public sealed class FontconfigPreferences
{
    /// <summary>The generic families whose preference lists are read.</summary>
    /// <remarks>
    /// CSS's generic families plus fontconfig's own additions. A preference expressed for any of
    /// them is a statement about what should draw text nothing else claims, which is exactly the
    /// question glyph fallback asks.
    /// </remarks>
    private static readonly HashSet<string> Generics = new(StringComparer.Ordinal)
    {
        "serif", "sans-serif", "sans serif", "sans", "monospace", "mono",
        "cursive", "fantasy", "system-ui", "emoji", "math",
    };

    /// <summary>How deep an <c>&lt;include&gt;</c> chain is followed.</summary>
    /// <remarks>
    /// A configuration that includes itself is malformed rather than impossible, and a stack
    /// overflow inside a font resolver is a poor way to find out.
    /// </remarks>
    private const int MaxIncludeDepth = 8;

    private readonly Dictionary<string, int> _ranks;

    private FontconfigPreferences(Dictionary<string, int> ranks) => _ranks = ranks;

    /// <summary>A preference set naming nothing, for a machine with no fontconfig at all.</summary>
    public static FontconfigPreferences None { get; } =
        new(new Dictionary<string, int>(StringComparer.Ordinal));

    /// <summary>The machine's own preferences, read once.</summary>
    /// <remarks>
    /// Lazily, because a process that renders nothing should not pay for a directory scan, and
    /// cached, because the answer cannot change while the process runs. A machine with no
    /// fontconfig — every Windows and most macOS installations — gets <see cref="None"/> and the
    /// resolver behaves exactly as it did before this existed.
    /// </remarks>
    public static FontconfigPreferences Machine => _machine.Value;

    private static readonly Lazy<FontconfigPreferences> _machine = new(Load);

    /// <summary>The preferred families, best first, as normalised names.</summary>
    public IReadOnlyList<string> InOrder =>
        _ranks.OrderBy(entry => entry.Value).Select(entry => entry.Key).ToList();

    /// <summary>
    /// Where a family sits in the preference order, or <see cref="int.MaxValue"/> when it is not
    /// named at all.
    /// </summary>
    /// <param name="familyName">A family name; normalised here, so either form is accepted.</param>
    public int RankOf(string? familyName)
        => _ranks.TryGetValue(FontSubstitutions.Normalise(familyName), out int rank)
            ? rank
            : int.MaxValue;

    /// <summary>Reads the machine's configuration from its usual place.</summary>
    /// <remarks>
    /// <c>FONTCONFIG_FILE</c> wins, then <c>$FONTCONFIG_PATH/fonts.conf</c>, then
    /// <c>/etc/fonts/fonts.conf</c> — the same order fontconfig itself uses, which matters because
    /// every comparison against a reference rendering has to be made against the configuration
    /// that produced it.
    /// </remarks>
    public static FontconfigPreferences Load()
    {
        foreach (string candidate in RootCandidates())
        {
            if (!File.Exists(candidate)) continue;

            Dictionary<string, int> ranks = new(StringComparer.Ordinal);
            ReadFile(candidate, ranks, 0);
            return new FontconfigPreferences(ranks);
        }

        return None;
    }

    /// <summary>Reads a set of configuration files in the order given, for tests.</summary>
    public static FontconfigPreferences Read(IEnumerable<string> files)
    {
        ArgumentNullException.ThrowIfNull(files);

        Dictionary<string, int> ranks = new(StringComparer.Ordinal);
        foreach (string file in files) ReadFile(file, ranks, 0);
        return new FontconfigPreferences(ranks);
    }

    private static IEnumerable<string> RootCandidates()
    {
        if (Environment.GetEnvironmentVariable("FONTCONFIG_FILE") is { Length: > 0 } named)
        {
            yield return Path.IsPathRooted(named) ? named : Path.Combine("/etc/fonts", named);
        }

        if (Environment.GetEnvironmentVariable("FONTCONFIG_PATH") is { Length: > 0 } directory)
        {
            yield return Path.Combine(directory, "fonts.conf");
        }

        yield return "/etc/fonts/fonts.conf";
    }

    private static void ReadFile(string path, Dictionary<string, int> ranks, int depth)
    {
        if (depth > MaxIncludeDepth) return;

        XDocument document;
        try
        {
            // No DTD resolution: every fontconfig file declares one and none of them needs it.
            using FileStream stream = File.OpenRead(path);
            document = XDocument.Load(stream, LoadOptions.None);
        }
        catch (Exception exception)
            when (exception is IOException or UnauthorizedAccessException
                      or System.Xml.XmlException)
        {
            // A configuration file that cannot be read leaves the machine with fewer preferences,
            // which is the state every non-Linux machine is in anyway.
            return;
        }

        if (document.Root is null) return;

        foreach (XElement element in document.Root.Elements())
        {
            switch (element.Name.LocalName)
            {
                case "alias":
                    ReadAlias(element, ranks);
                    break;
                case "include":
                    foreach (string included in Included(element, path))
                    {
                        ReadFile(included, ranks, depth + 1);
                    }

                    break;
            }
        }
    }

    private static void ReadAlias(XElement alias, Dictionary<string, int> ranks)
    {
        // The subject is the alias's own `family` child; the preferences are inside `prefer`.
        string? subject = alias.Elements()
            .FirstOrDefault(child => child.Name.LocalName == "family")?.Value.Trim();

        if (subject is null || !Generics.Contains(subject)) return;

        foreach (XElement prefer in alias.Elements().Where(e => e.Name.LocalName == "prefer"))
        {
            foreach (XElement family in prefer.Elements().Where(e => e.Name.LocalName == "family"))
            {
                string normalised = FontSubstitutions.Normalise(family.Value.Trim());
                if (normalised.Length == 0) continue;

                // First mention wins: a family named by two files takes the better rank, which is
                // what prepending at the matched family's position produces.
                if (!ranks.ContainsKey(normalised)) ranks[normalised] = ranks.Count;
            }
        }
    }

    private static IEnumerable<string> Included(XElement include, string from)
    {
        string target = include.Value.Trim();
        if (target.Length == 0) yield break;

        if (target.StartsWith('~'))
        {
            target = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.UserProfile),
                target.TrimStart('~', '/'));
        }
        else if (!Path.IsPathRooted(target))
        {
            target = Path.Combine(Path.GetDirectoryName(from) ?? ".", target);
        }

        if (Directory.Exists(target))
        {
            string[] files;
            try
            {
                files = Directory.GetFiles(target, "*.conf");
            }
            catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
            {
                yield break;
            }

            // By name, ascending: fontconfig reads a configuration directory in exactly that order
            // and the numeric prefixes on the files exist to control it.
            Array.Sort(files, StringComparer.Ordinal);
            foreach (string file in files) yield return file;
        }
        else if (File.Exists(target))
        {
            yield return target;
        }
    }
}
