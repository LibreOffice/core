#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

// -----------------------------------------------------------------------------
// taken from setup2
std::string getTempPath()
{
    std::string sTempDir;

    char* pTmp = getenv( "TEMP" );
    if (pTmp == NULL) pTmp = getenv("temp");
    if (pTmp == NULL) pTmp = getenv("TMP");
    if (pTmp == NULL) pTmp = getenv("tmp");

    if( pTmp && strlen(pTmp) >= 2 )
    {
        sTempDir = std::string( pTmp );
    }
    else
    {
#ifdef UNX
        int nLen;
        pTmp = P_tmpdir;
        nLen = strlen(pTmp);
        if (pTmp[ nLen - 1] == '/')
        {
            char cBuf[256];
            char* pBuf = cBuf;
            strncpy( pBuf, pTmp, nLen - 1 );
            pBuf[nLen - 1] = '\0';
            sTempDir = std::string( pBuf );
        }
        else
        {
            sTempDir = std::string( pTmp );
        }
#else
        fprintf(stderr, "error: No temp dir found.\n");
#endif
    }
    return sTempDir;
}
