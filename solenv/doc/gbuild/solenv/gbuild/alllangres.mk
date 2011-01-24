#include <types.h>

namespace gb
{
    class SrsTarget;
    class ResTarget;
    class AllLangResTarget;

    class SrsPartTarget : public Target, public HasDependencies, public UsesRsc
    {
        private:
            /// SrsPartTarget s do not need to be explicitly constructed.

            /// They are a helper class for the SrsTarget class.
            /// They are named after the path of their source file (without
            /// file extension) from the root of their source repository.
            SrsPartTarget(String name);
            friend class SrsTarget;

            /// Platformdependant command to generate the dependencies for the srs target.
            static const Command command_dep(
                String srsname,
                Path sourcefile,
                List<Path> include,
                List<String> defs);
    };

    class SrsTarget : public Target, public HasDependencies, public IsCleanable
    {
        public:
            /// Sets defines for srs processing.
            /// \$\$(DEFS) contains the current flags and can be used, if
            /// just a few need to be modified.
            void set_defs(List<String> defs);
            /// Sets the include paths for srs processing.
            /// \$\$(INCLUDE) contains the current paths and can be used, if
            /// just a few need to be modified.
            void set_include(List<String> include);
            /// Adds a SrsPartTarget to be processed.
            void add_file(SrsPartTarget part);
            /// Adds multiple SrsPartTarget s to be processed.
            void add_files(List<SrsPartTarget> parts);
        private:
            /// SrsTarget s do not need to be explicitly constructed.

            /// They are a helper class for the ResTarget class.
            SrsTarget(String name);
            friend class ResTarget;

            List<String> DEFS;
            List<String> INCLUDE
            List<SrsPartTarget> PARTS;
            static const List<String> DEFAULTDEFS;
    };

    class ResTarget : public Target, public IsCleanable, public HasSrs
    {
        public:
            void add_file(Path file);
            void add_files(List<Path> files);
        private:
            /// ResTarget do not need to be explicitly constructed.

            /// They are a helper class for the AllLangResTarget class.
            ResTarget(String name, String library, Language lang, Path reslocation);
            friend class AllLangResTarget;

            String LIBRARY;
            Language LANGUAGE;
            Path RESLOCATION;
    };

    class AllLangResTarget : public Target, public IsCleanable, public HasSrs
    {
        public:
            AllLangResTarget(String name);
            /// Sets the location where resources are to be found. reslocation is a path relative to the default_images directory.
            /// Resources will be found in reslocation itself and in the subdirectories res/ and imglst/ of it and again in subdirectories
            /// of those named as the language of the processed resource. In addition, resources are found in the res/ directory directly
            /// below default_images and the subdirectory of it named as the language of the processed resource.
            void set_reslocation(Path reslocation);
        private:
            /// Sets the languages for which to create resources.
            static void set_langs(List<Language> langs);

            /// The list of languages that need to be build.
            static List<Language> LANGS;
    };
}
/* vim: set filetype=cpp : */
