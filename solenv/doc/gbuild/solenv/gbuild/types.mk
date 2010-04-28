namespace gb {
    class CObject;
    class CxxObject;
    class Library;
    class StaticLibrary;
    class Package;
    class SdiTarget;
};

namespace gb { namespace types
{
    /// A plain old string.
    class String {};
    /// A partial, relative or absolute filesystem path.
    class Path {};
    /// A target to be build.
    class Target
    {
        public:
            /// The absolute filesystem path representing the target.
            Path get_target();
    };
    /// A target that can be linked against statically.
    class StaticLinkable {};
    /// A partial or complete shell-command.
    class Command {};
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

    /// A Mapping from with a key of type K and a value of type V

    /// A Mapping is represented by a whitespace separated list
    /// of key-value pairs. Key and value are separated by a colon (:).
    template <typename K, typename V>
        class Map<K,V> {};
    /// A target that can be cleaned.
    class IsCleanable
    {
        public:
            /// The (phony) absolute filesystem path to clean the target.
            Path get_clean_target();
    };
    /// A target that has generated dependencies.
    class HasDependencies
    {
        public:
            /// The absolute filesystem path to the file containing the dependencies.
            Path get_dep_target();
    };
    /// A target that has a source file from which it is generated.
    class HasSource
    {
        public:
            /// The absolute filesystem path to the source file.
            Path get_source();
    };
    /// A target that links objects and libraries.
    class IsLinking
    {
        public:
            /// Add a CObject to be compiled and linked.
            void add_cobject(const CObject& cobject);
            /// Add multiple CObject s to be compiled and linked.
            void add_cobjects(const List<CObject>& cobjects);
            /// Add a CxxObject to be compiled and linked.
            void add_cxxobject(const CxxObject& cobject);
            /// Add multiple CxxObject s to be compiled and linked.
            void add_cxxobjects(const List<CxxObject>& cobjects);
            /// Add multiple CxxObject s to be compiled and linked (with exceptions enabled).
            void add_exception_objects(const List<CxxObject>& cobject);
            /// Add libraries to link against dynamically.
            void add_linked_libs(const List<Library>& linked_libs);
            /// Add libraries to link against statically.
            void add_linked_static_libs(const List<StaticLibrary>& linked_static_libs);
            /// Add multiple CxxObject s to be compiled and linked (without exceptions enabled).
            /// @deprecated { We should not care about disabling exception. }
            void add_noexception_objects(const List<CxxObject>& cobject);
            /// Set auxiliary files that are produced by linking (for cleanup and copying).
            void set_auxtargets(const List<Path>& auxtargets);
            /// Set the location for the produced DLL (used on Windows only).
            void set_dll_target(const Path& dlltarget);
            /// Set additional flags for the link command.
            void set_ldflags(const List<Path>& ldflags);
    };
    /// A target that delivers headers of some kind.
    class DeliversHeaders
    {
        public:
            /// The absolute filesystem path which is touched when all headers for this target are prepared.
            Path get_headers_target();
            /// The absolute filesystem path which is touched when all external headers for this target are prepared.
            Path get_external_headers_target();
            /// Add multiple Packages that need to be delivered/generated
            /// before compilation or dependency generation can start.
            void add_package_headers(const List<Package>& packages);
            /// Add multiple SdiTargets that need to be delivered/generated
            /// before compilation or dependency generation can start.
            void add_sdi_headers(const List<SdiTarget>& sdis);
    };
    /// A target where settings for the compilation can be set.
    class HasCompileSettings
    {
        public:
            /// Sets flags for plain C compilation.
            /// \$\$(CFLAGS) contains the current flags and can be used, if
            /// just a few need to be modified.
            void set_cflags(const List<String>& cflags);
            /// Sets flags for C++ compilation.
            /// \$\$(CXXFLAGS) contains the current flags and can be used, if
            /// just a few need to be modified.
            void set_cxxflags(const List<String>& cxxflags);
            /// Sets defines for C/C++ compilation.
            /// \$\$(DEFS) contains the current flags and can be used, if
            /// just a few need to be modified.
            void set_defs(const List<String>& defs);
            /// Sets the include paths for C/C++ compilation.
            /// \$\$(INCLUDE) contains the current paths and can be used, if
            /// just a few need to be modified.
            void set_include(const List<Path>& include);
            /// Sets the stl include paths for C++ compilation.
            /// \$\$(INCLUDE_STL) contains the current paths and can be used, if
            /// just a few need to be modified.
            void set_include_stl(const List<Path>& include_stl);
    };
}};
/* vim: set filetype=cpp : */
