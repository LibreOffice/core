
#include <types.mk>
using namespace gb::types;

// GNU make specific setup
static const Command SHELL;


// gbuild root directorys
static Path SRCDIR;
static Path OUTDIR;
static Path WORKDIR;

// Expected from configure/environment
static const Integer OSL_DEBUG_LEVEL;
static const List<Path> JDKINCS;
static const List<Path> SOLARINC;
static const List<Path> SOLARLIB;
static const Path GBUILDDIR;
static const Path INPATH;
static const Path JAVA_HOME;
static const Path UPD;
static const String CVER;
static const String LIBXML_CFLAGS;
static const String OS;
static const String STLPORT_VER;
static const Bool DEBUG;
static const Bool PRODUCT;

// gbuild global variables derived from the configure/environment
// some of these are defined per platform
namespace gb
{
    /// building with generated dependencies
    static const Bool FULLDEPS;
    /// product build or non-product build with additional assertions and tests
    static const Bool PRODUCT;
    /// command to run awk scripts
    static const Command AWK;
    /// command to compile c source files
    static const Command CC;
    /// command to compile c++ source files
    static const Command CXX;
    /// command to process input with a gcc compatible preprocessor
    static const Command GCCP;
    /// command to link objects on the microsoft toolchain
    static const Command LINK;
    /// command to create a unique temporary file
    static const Command MKTEMP;
    /// debuglevel:
    /// 0=no debugging,
    /// 1=non-product build,
    /// 2=debugging build (either product or nonproduct)
    static const Integer DEBUGLEVEL;
    /// compiler specific optimization flags
    static const List<String> COMPILEROPTFLAGS;
    /// default c compilation compiler flags
    static const List<String> CFLAGS;
    /// compiler specific defines
    static const List<String> COMPILERDEFS;
    /// cpu-specific default defines
    static const List<String> CPUDEFS;
    /// default c++ compilation compiler flags
    static const List<String> CXXFLAGS;
    /// platform- and compilerindependent default defines
    static const List<String> GLOBALDEFS;
    /// default objective c++ compilation compiler flags
    static const List<String> OBJCXXFLAGS;
    /// platfomspecific default defines
    static const List<String> OSDEFS;
    /// ?
    static const Path SDKDIR;
};

// GXX_INCLUDE_PATH (Linux)
// PTHREAD_CFLAGS (Linux)
// SYSTEM_ICU (Linux)
// SYSTEM_JPEG (Linux)
// SYSTEM_LIBXML (Linux)
// USE_SYSTEM_STL (Linux)

/* vim: set filetype=cpp : */
