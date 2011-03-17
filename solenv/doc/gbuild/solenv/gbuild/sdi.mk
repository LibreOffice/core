#include <types.h>

namespace gb
{
    using namespace types;

    class SdiTarget : public Target, public IsCleanable
    {
        public:
            /// Creates a new SdiTarget. 
            SdiTarget(String name, String exports);
            /// Sets the include paths for this SdiTarget.
            set_include(List<Path> includes);
        private:
            /// The command to execute svidl.
            static const Command SVIDLCOMMAND;
            /// The target on with to depend to make sure the svidl executable is available.
            static const Path SVIDLTARGET;
            /// The target on with to depend to make sure the auxiliary files (libraries etc.) for the svidl executable are available.
            static const Path SVIDLAUXDEPS;
    };
}
/* vim: set filetype=cpp : */
