using System.Collections.Concurrent;

namespace XRay.Office.Core.Globalization;

/// <summary>
/// Where hyphenation pattern files come from: one shipped, the rest supplied.
/// </summary>
/// <remarks>
/// <para>
/// <strong>American English travels with the library and nothing else does.</strong>
/// <c>hyph_en_US.dic</c> is BSD-style — <em>"Unlimited copying, redistribution and modification
/// of this file is permitted with this copyright and license information"</em> — so it ships,
/// with <c>README_hyph_en_US.txt</c> beside it because the notice lives in the README and not in
/// the data. The other three files LibreOffice distributes are not on those terms:
/// <c>hyph_fr.dic</c> is LGPL-2.1-or-later and <c>hyph_es.dic</c> a GPL-3.0+/LGPL-3.0+/MPL-1.1+
/// tri-licence, and shipping either would change this package's licensing position. They are
/// therefore <em>supplied</em>, not bundled, and everything below exists to make supplying them
/// a supported operation rather than a patch.
/// </para>
/// <para>
/// <strong>Why one file is enough for the default.</strong> Measured over the corpus'
/// 766 OOXML containers (<c>probes/chart-hyph-r105</c>): 103 state a category axis, 102 of them
/// carry pure-ASCII English labels, and the one French document's axis does not move whether the
/// reference hyphenates it or not. 23 of the 103 state no language at all, and LibreOffice
/// 26.2.4.2 hyphenates those with its own default locale — which is why
/// <see cref="DefaultLanguage"/> exists and is <c>en-US</c> rather than "no hyphenation".
/// </para>
/// <para>
/// <strong>Three ways to add one, in the order they are consulted.</strong>
/// </para>
/// <list type="number">
///   <item><description><see cref="Register(string, IHyphenator)"/> — an in-process override,
///     which is also how a stream becomes a hyphenator:
///     <c>Register("fr-FR", HyphenationPatterns.Read(stream))</c>. Nothing has to be on
///     disk.</description></item>
///   <item><description>Directories named in <see cref="Variable"/>, separated by the platform's
///     path separator. Each is searched for <c>hyph_&lt;tag&gt;.dic</c> and then in its immediate
///     subdirectories, so pointing at a LibreOffice installation's
///     <c>share/extensions</c> finds <c>dict-fr/hyph_fr.dic</c> without naming every
///     one.</description></item>
///   <item><description>The <c>hyphenation</c> folder beside the assembly, which is where the
///     shipped file lands.</description></item>
/// </list>
/// <para>
/// <see cref="Variable"/> set to <c>0</c>, <c>false</c> or <c>no</c> switches hyphenation off
/// altogether, which is the state a deployment without the data is in and therefore the state
/// worth being able to reproduce on a machine that has it. Read that way round — an unrecognised
/// value leaves the shipped file in place — so a variable set to something unexpected cannot
/// silently change what a document renders as. The bundled faces in <c>XRay.Office.Text</c> use the
/// same shape (<c>XRAY_OFFICE_BUNDLED_FONTS</c>).
/// </para>
/// <para>
/// <strong>The default must not depend on a LibreOffice installation.</strong> Nothing here
/// looks in <c>/opt</c> or <c>/usr/share/hyphen</c> on its own. A machine with Hunspell
/// dictionaries installed gets them by being told where they are; a machine without them gets
/// English and no more, which is a floor rather than a lottery.
/// </para>
/// </remarks>
/// <seealso cref="HyphenationPatterns"/>
public static class Hyphenators
{
    /// <summary>The variable that adds search directories, or turns hyphenation off.</summary>
    public const string Variable = "XRAY_OFFICE_HYPHEN_DICTS";

    /// <summary>The folder the shipped pattern file is copied into, beside the assembly.</summary>
    public const string FolderName = "hyphenation";

    /// <summary>
    /// What a run that states no language is hyphenated as.
    /// </summary>
    /// <remarks>
    /// Measured rather than chosen: 23 of the corpus' 103 category-axis documents state no
    /// language on their chart text, the witness <c>038_Competitive_Advantage_Card</c> among
    /// them, and LibreOffice 26.2.4.2 hyphenates all of them — retagging the same runs to a
    /// locale it has no patterns for is what makes it stop (<c>probes/chart-hyph-r105</c> §1.1).
    /// Which setting supplies its default was not isolated; English (USA) is what it behaves as
    /// here, and a deployment whose default differs would differ.
    /// </remarks>
    public const string DefaultLanguage = "en-US";

    private static readonly ConcurrentDictionary<string, IHyphenator?> Found = new(StringComparer.OrdinalIgnoreCase);

    private static readonly ConcurrentDictionary<string, IHyphenator> Supplied = new(StringComparer.OrdinalIgnoreCase);

    private static string? Setting => Environment.GetEnvironmentVariable(Variable);

    /// <summary>Whether any pattern file may be used at all.</summary>
    public static bool Enabled => Setting is not ("0" or "false" or "no");

    /// <summary>A hyphenator that never finds a point — the no-dictionary state, named.</summary>
    /// <remarks>
    /// Worth a type of its own because it is the state a deployment without the data is in, so it
    /// is what a test of the default behaviour has to be able to ask for explicitly rather than
    /// arrange by deleting a file.
    /// </remarks>
    public static IHyphenator None { get; } = new NoHyphenator();

    /// <summary>
    /// The directories searched for <c>hyph_*.dic</c>, in order: those named in
    /// <see cref="Variable"/>, then the shipped folder.
    /// </summary>
    public static IReadOnlyList<string> SearchPath
    {
        get
        {
            if (!Enabled) return [];

            List<string> path = [];

            if (Setting is { Length: > 0 } setting)
            {
                foreach (string entry in setting.Split(Path.PathSeparator))
                    if (entry.Length > 0 && Directory.Exists(entry)) path.Add(entry);
            }

            if (Shipped() is { } shipped) path.Add(shipped);

            return path;
        }
    }

    /// <summary>
    /// The hyphenator for a language, or null when no pattern file answers for it.
    /// </summary>
    /// <param name="language">
    /// A BCP 47 tag, or null or empty for <see cref="DefaultLanguage"/>.
    /// </param>
    /// <remarks>
    /// Resolved once per tag and cached, including the negative answer: the search walks
    /// directories, and a chart axis asks this once per label.
    /// </remarks>
    public static IHyphenator? For(string? language)
    {
        if (!Enabled) return null;

        string tag = string.IsNullOrWhiteSpace(language) ? DefaultLanguage : language.Trim();

        if (Supplied.TryGetValue(tag, out IHyphenator? registered)) return registered;

        return Found.GetOrAdd(tag, Locate);
    }

    /// <summary>Supplies a hyphenator for a language, overriding anything on disk.</summary>
    /// <remarks>
    /// The seam for a caller that has the patterns as a stream, a resource or an implementation
    /// of its own. It is process-wide, like the search path, because the alternative is threading
    /// a hyphenator through every layout call for the sake of a setting that never varies within
    /// a process.
    /// </remarks>
    public static void Register(string language, IHyphenator hyphenator)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(language);
        ArgumentNullException.ThrowIfNull(hyphenator);

        Supplied[language.Trim()] = hyphenator;
        Found.Clear();
    }

    /// <summary>Reads a pattern file and supplies it for a language.</summary>
    public static void Register(string language, Stream patterns)
        => Register(language, HyphenationPatterns.Read(patterns));

    /// <summary>Drops everything <see cref="Register(string, IHyphenator)"/> supplied.</summary>
    /// <remarks>
    /// For a test that registered one: the registry is process-wide, so a test that leaves a
    /// French hyphenator behind changes what the next test measures.
    /// </remarks>
    public static void Forget()
    {
        Supplied.Clear();
        Found.Clear();
    }

    /// <summary>
    /// The file names a tag is looked for under, most specific first.
    /// </summary>
    /// <remarks>
    /// <c>fr-FR</c> looks for <c>hyph_fr_FR.dic</c> and then <c>hyph_fr.dic</c>, which is how
    /// LibreOffice's own files are named — <c>dict-fr/hyph_fr.dic</c> against
    /// <c>dict-en/hyph_en_US.dic</c>, one with a region and one without.
    /// </remarks>
    internal static IEnumerable<string> Names(string tag)
    {
        string[] parts = tag.Split('-', '_');
        string language = parts[0].ToLowerInvariant();

        if (parts.Length > 1)
        {
            yield return language + "_" + parts[^1].ToUpperInvariant();
            yield return language + "_" + parts[^1];
        }

        yield return language;
    }

    private static IHyphenator? Locate(string tag)
    {
        foreach (string directory in SearchPath)
        {
            foreach (string name in Names(tag))
            {
                if (File.Exists(Path.Combine(directory, "hyph_" + name + ".dic"))
                    && Read(Path.Combine(directory, "hyph_" + name + ".dic")) is { } here)
                {
                    return here;
                }

                // One level down as well: a LibreOffice installation keeps each language in its
                // own dict-xx folder, so the useful thing to be handed is the folder above them.
                foreach (string nested in Subdirectories(directory))
                {
                    string candidate = Path.Combine(nested, "hyph_" + name + ".dic");
                    if (File.Exists(candidate) && Read(candidate) is { } below) return below;
                }
            }
        }

        return null;
    }

    private static IEnumerable<string> Subdirectories(string directory)
    {
        try
        {
            return Directory.EnumerateDirectories(directory);
        }
        catch (IOException)
        {
            return [];
        }
        catch (UnauthorizedAccessException)
        {
            return [];
        }
    }

    private static HyphenationPatterns? Read(string path)
        => HyphenationPatterns.ReadFile(path) is { Count: > 0 } read ? read : null;

    private static string? Shipped()
    {
        // AppContext.BaseDirectory rather than the assembly's own location: the two differ for a
        // single-file publish, and it is the base directory content files are copied beside.
        string? baseDirectory = AppContext.BaseDirectory;
        if (string.IsNullOrEmpty(baseDirectory)) return null;

        string candidate = Path.Combine(baseDirectory, FolderName);
        return Directory.Exists(candidate) ? candidate : null;
    }

    private sealed class NoHyphenator : IHyphenator
    {
        public IReadOnlyList<int> FindHyphenationPoints(ReadOnlySpan<char> word, string language)
            => [];
    }
}
