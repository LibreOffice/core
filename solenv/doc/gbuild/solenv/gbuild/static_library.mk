#include <types.h>

namespace gb
{
    using namespace types;

    class LinkTarget;

    class StaticLibrary : public IsCleanable, public HasDependencies, public IsLinking, public DeliversHeaders, public HasCompileSettings, public Target
    {
        public:
            StaticLibrary(String name);
            /// Returns how the library is named in the filesystem on the current platform.
            /// StaticLibrary filename schemes are defined in solenv/gbuild/libnames.mk (with some platformdependant hacks in solenv/gbuild/platform/*.mk.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_filename();

        private:
            /// private helper function for the constructor
            void StaticLibrary_impl(LinkTarget library_linktarget);
            /// helper function to wrap LinkTargets functions (this is more or less pimpl ...).
            void forward_to_linktarget(Function f);

            /// List of buildable static libraries (i.e. static libraries that are not expected to exist outside of \$(OUTDIR) on the system).
            static const List<StaticLibrary> TARGETS;
            /// List of additional defines for compiling objects for static libraries
            static const List<String> DEFS;
            /// List of additional flags for linking a static library
            static const List<String> TARGETTYPEFLAGS;
            /// Mapping from symbolic static library names to filenames
            static const Map<StaticLibrary,Path> FILENAMES;
            /// location to place static libraries in the \$(OUTDIR)
            static const Path OUTDIRLOCATION;
            /// platformdependant file extension for static libraries
            static const String PLAINEXT;
    };
}
/* vim: set filetype=cpp : */
