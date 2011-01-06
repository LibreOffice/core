#include <types.h>

namespace gb
{
    using namespace types;
    class PackagePart
    {
        private:
            /// PackagePart s do not need to be explicitly constructed.
            /// They are named after the path of their source file (without
            /// file extension) from the root of their source repository.
            PackagePart(String name, Path Source);
            friend Package;
            /// Returns a list of aboslute paths where files are copied to with the PackagePart class.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            static const List<Path> get_destinations();
    };

    class Package : public Target, public IsCleanable
    {
        public:
            /// Creates a new package that copies files from source_dir to the \$(OUTDIR).
            Package(String name, Path source_dir);
            /// Adds a file to the package. destination is the relative path in
            /// the \$(OUTDIR) and source is the relative path in the source_dir.
            add_file(Path destination, Path source);
    }
}
/* vim: set filetype=cpp : */
