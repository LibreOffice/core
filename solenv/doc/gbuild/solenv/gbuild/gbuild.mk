
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
namespace gb
{
    static const Integer PRODUCT;
    static const Integer DEBUGLEVEL;
    static const Bool FULLDEPS;
    static const List<String> GLOBALDEFS;
};

// GXX_INCLUDE_PATH (Linux)
// PTHREAD_CFLAGS (Linux)
// SYSTEM_ICU (Linux)
// SYSTEM_JPEG (Linux)
// SYSTEM_LIBXML (Linux)
// USE_SYSTEM_STL (Linux)

/* vim: set filetype=cpp : */
