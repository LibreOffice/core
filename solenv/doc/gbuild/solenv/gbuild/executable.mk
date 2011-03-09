#include <types.h>

namespace gb
{
    using namespace types;

    class LinkTarget;

    class Executable : public IsCleanable, public HasDependencies, public IsLinking, public DeliversHeaders, public HasCompileSettings, public Target
    {
        public:
            Executable(String name);

        private:
            /// private helper function for the constructor
            void Executable_impl(LinkTarget library_linktarget);
            /// platformdependant additional setup for constructor (used on Windows only)
            void Executable_platform(LinkTarget library_linktarget);
            /// helper function to wrap LinkTargets functions (this is more or less pimpl ...)
            void forward_to_linktarget(Function f);

            static const List<String> TARGETTYPEFLAGS;
    };
}
/* vim: set filetype=cpp : */
