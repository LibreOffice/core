#ifndef _XMLBUILDLISTDEF_HXX
#define _XMLBUILDLISTDEF_HXX

enum DepTypes
{
    mdsimple,
    mdalways,
    mdforce
};

#define DEP_MD_SIMPLE   1;
#define DEP_MD_ALWAYS   2;
#define DEP_MD_FORCE    4;

static const char * DEP_MD_ALWAYS_STR = "md-always";
static const char * DEP_MD_FORCE_STR  = "md-force";

#endif


