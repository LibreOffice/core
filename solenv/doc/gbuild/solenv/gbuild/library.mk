#include <types.h>

namespace gb
{
    using namespace types;

    class LinkTarget;

    class Library : public IsCleanable, public HasDependencies, public IsLinking, public DeliversHeaders, public HasCompileSettings, public Target
    {
        public:
            Library(String name);
            /// Returns how the library is named in the filesystem on the current platform.
            /// Library filename schemes are defined in solenv/gbuild/libnames.mk (with some platformdependant hacks in solenv/gbuild/platform/*.mk.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_filename();

        private:
            /// private helper function for the constructor
            void Library_impl(LinkTarget library_linktarget);
            /// platformdependant additional setup for constructor (used on Windows only)
            void Library_platform(
                LinkTarget library_linktarget,
                Path dllfile);
            /// helper function to wrap LinkTargets functions (this is more or less pimpl ...)
            void forward_to_linktarget(Function f);

            /// List of buildable libraries (i.e. libraries that are not expected to exist outside of \$(OUTDIR) on the system).
            static const List<Library> TARGETS;
            /// List of additional defines for compiling objects for libraries
            static const List<String> DEFS;
            /// List of additional flags for linking a library
            static const List<String> TARGETTYPEFLAGS;
            /// Mapping from symbolic library names to dll filenames
            static const Map<Library,Path> DLLFILENAMES;
            /// Mapping from symbolic library names to filenames
            static const Map<Library,Path> FILENAMES;
    };
}
/* vim: set filetype=cpp : */
