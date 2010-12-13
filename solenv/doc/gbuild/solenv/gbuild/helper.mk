#include <types.h>

String NEWLINE;
String COMMA;

namespace gb
{
    using namespace types;

    class Helper
    {
        public:
            /// Abbreviates the common directory roots in a command
            static Command abbreviate_dirs(Command c);
            /// Announces the start/end of an task.
            static void announce(String announcement);
            /// Creates the default get_clean_target() function for the type.
            static void make_clean_target(TargetType t);
            /// Creates the default get_clean_target() function for the types.
            static void make_clean_targets(List<TargetType> ts);
            /// Creates the default get_dep_target() function for the type.
            static void make_dep_target(TargetType t);
            /// Creates the default get_dep_target() function for the types.
            static void make_dep_targets(List<TargetType> ts);
            /// Creates the default get_dep_target() function for the type modifying the \$(OUTDIR).
            static void make_outdir_clean_target(TargetType t);
            /// Creates the default get_dep_target() function for types modifying the \$(OUTDIR).
            static void make_outdir_clean_targets(List<TargetType> ts);
            /// Returns the clean target of a target modifying the \$(OUTDIR).
            static Path get_outdir_clean_target(Path target);

            /// The path to a zero length file.
            static const Path NULLFILE;
            /// The path to the misc directory for temporary files etc.
            static const Path MISC;
            /// The path to a phony target, that is always outdated.
            static const Path PHONY;
            /// Internally used dummy file. 
            static const Path MISCDUMMY;
    };
}
/* vim: set filetype=cpp : */
