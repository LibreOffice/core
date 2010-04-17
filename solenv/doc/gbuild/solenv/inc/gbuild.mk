

namespace types
{
    /// A plain old string.
    class String {};
    /// A partial, relative or absolute filesystem path.
    class Path {};
    /// A target represented by an absolute filesystem path.
    class Target {};
    /// A partial or complete shell-command.
    class Command {};
    /// A library name as used on in the linking step.
    class Linkname {};
    /// A integer number.
    class Integer {};
    /// A boolean value.

    /// There are const boolean values true and false for
    /// comparisons (written as $(true) and $(false) in makefiles.
    class Bool {};
    /// A language (for localisation)

    /// A language represented by its ISO 639-1:2002 code.
    class Language {};
    /// A List of objects.

    /// A List of objects represented by a whitespace separated list
    /// in makefiles.
    template <typename T>
        class List<T> {};
};

using namespace types;

namespace gb
{
    class Library;
    class OOoLibrary;
    class RtLibrary;
    class Executable;
    class Module;
    class AllLangResTarget;
    class SystemLibrary {};

    class Helper
    {
        public:
            static Command announce(Target target);
            static Command abbreviate_dirs(Command command);
    };

    class StaticLibrary
    {
        public:
            StaticLibrary(String name);
            Target get_target();
    };

    class Objectfile
    {
        public:
            Objectfile(String name);
            Target get_target();
            Target get_dep_filename();
            Target get_cxx_source_filename();
    };

    enum LinkTargetType {
        LIB,
        EXE
    };

    class LinkTarget
    {
        private:
            friend class Library;
            friend class Executable;

            LinkTarget(LinkTargetType type, String name, Target target);
            Target get_target();
            Target get_headers_target();
            Target get_dep_filename();
            void set_cxxflags(String cxxflags);
            void set_visibility_hidden(Bool visibility_hidden);
            void set_include(String include);
            void set_ldflags(String ldflags);
            void set_library_path_flags(String library_path_flags);
            void add_linked_libs(List<Library> linked_libraries);
            void add_object(Objectfile objectfile);
            void add_noexception_object(Objectfile objectfile);
            void add_exception_object(Objectfile objectfile);
            void add_noexception_objects(List<Objectfile> objectfiles);
            void add_exception_objects(List<Objectfile> objectfiles);

            List<String> DEFS;
            List<String> CXXFLAGS;
            List<String> INCLUDE;
            String VISIBILITY;
            List<String> LDFLAGS;
            List<String> LIBRARY_PATH_FLAGS;
            List<Linkname> LINKED_LIBS;
            List<Linkname> LINKED_SYS_LIBS;
            List<Objectfile> OBJECTS;
            List<String> TARGETTYPE_LINK_FLAGS;
    };


    class Library
    {
        protected:
            Library(String name, Target target);
        public:
            Target get_target();
            Target get_headers_target();
            Target get_dep_filename();
            void set_cxxflags(String cxxflags);
            void set_visibility_hidden(Bool visibility_hidden);
            void set_include(String include);
            void set_ldflags(String ldflags);
            void set_library_path_flags(String library_path_flags);
            void add_linked_libs(List<Library> linked_libraries);
            void add_linked_system_libs(List<SystemLibrary> linked_system_libraries);
            void add_object(Objectfile objectfile);
            void add_noexception_object(Objectfile objectfile);
            void add_exception_object(Objectfile objectfile);
            void add_noexception_objects(List<Objectfile> objectfiles);
            void add_exception_objects(List<Objectfile> objectfiles);
    };

    class OOoLibrary : public Library
    {
        public:
            OOoLibrary(String name);
            Target get_target();
            Target get_headers_target();
            Target get_dep_filename();
            Linkname get_linkname();
            static List<Linkname> get_linknames(List<OOoLibrary> ooolibraries);
    };

    class RtLibrary : public Library
    {
        public:
            RtLibrary(String name);
            Target get_target();
            Target get_headers_target();
            Target get_dep_filename();
            Linkname get_linkname();
            static List<Linkname> get_linknames(List<RtLibrary> ooolibraries);
    };

    class Executable
    {
        public:
            Executable(String name);
            Target get_target();
            void set_cxxflags(List<String> cxxflags);
            void set_visibility_hidden(Bool visibility_hidden);
            void set_include(List<String> include);
            void set_ldflags(List<String> ldflags);
            void set_library_path_flags(List<String> library_path_flags);
            void add_linked_libs(List<Library> linked_libraries);
            void add_linked_system_libs(List<SystemLibrary> linked_system_libraries);
            void add_object(Objectfile objectfile);
            void add_noexception_object(Objectfile objectfile);
            void add_exception_object(Objectfile objectfile);
            void add_noexception_objects(List<Objectfile> objectfiles);
            void add_exception_objects(List<Objectfile> objectfiles);
    };

    class SdiTarget
    {
        private:
            List<String> INCLUDE;
        public:
            SdiTarget(String name);
            Target get_target();
            void set_include(List<String> include);
    };

    class SrsTarget
    {
        private:
            List<String> DEFS;
            List<String> INCLUDE;
        public:
            SrsTarget(String name);
            Target get_target();
            void set_defs(List<String> defs);
            void set_include(List<String> include);
    };

    class ResTarget
    {
        private:
            friend class AllLangResTarget;
            ResTarget(String name, Language language);
        public:
            Target get_target();
            void add_file(SrsTarget srstarget);
            void add_files(List<SrsTarget> srstargets);
    };

    class AllLangResTarget
    {
        public:
            AllLangResTarget(String name);
            void add_file(SrsTarget srstarget);
            void add_files(List<SrsTarget> srstargets);
    };

    class Package
    {
        public:
            Package(String name, Path sourcedir);
            void add_file(Target destination, Path source);
            Path SOURCEDIR;
    };

    class Module
    {
        public:
            Module(String name);
            Target get_target();
            void make_global_targets();
            void read_includes(List<String> includes);
    };
};
static String GUI;
static String COM;
static Path INPATH;
static Path SRCDIR;
static Path OUTDIR;
static Path WORKDIR;
static bool FULL_DEPENDENCIES;
static List<gb::Module> ALL_MODULES;

static Command SHELL;
static Path CXX;
static Path GCCP;
static Target SVIDLTARGET;
static Command SVIDLCOMMAND;
static Target RSCTARGET;
static Command RSCCOMMAND;

static List<String> GLOBALDEFS;
static List<String> CFLAGSCXX;
static List<String> EXCEPTIONFLAGS;
static List<String> NOEXCEPTIONFLAGS;
static List<String> LDFLAGS;
static Integer OSL_DEBUG_LEVEL;

/* vim: set filetype=cpp : */
