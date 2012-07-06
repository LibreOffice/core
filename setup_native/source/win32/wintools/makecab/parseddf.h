/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __PARSEDDF_H__
#define __PARSEDDF_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <windows.h>
#include <fci.h>

typedef enum DDFLINETYPE { DDF_NONE, DDF_CMD, DDF_SRC } DDFLINETYPE;
typedef enum DDFERR { DDF_OK, DDFERR_INVALID_ENDL, DDFERR_NO_MORE_TOKENS, DDFERR_FUNC_FAILED, DDFERR_SRC_SKIPPED, DDFERR_UNREAD } DDFERR;
typedef enum DDFVAR { DDF_CABNAME, DDF_RESERVEPERCAB, DDF_MAXDISK, DDF_COMPTYPE, DDF_COMPRESS, DDF_COMPMEM, DDF_CABINET, DDF_DISKDIRTEMPLATE } DDFVAR;
typedef enum DDFLOGLEVEL { DDFLOG_ERR = 1, DDFLOG_WRN, DDFLOG_MSG } DDFLOGLEVEL;

#define DDF_MAX_CHARS 1024
#define DDF_MAX_LINES 1024
#define DDF_MAX_CABRESERVE 60000
#define DDF_MAX_CABNAME 50

typedef struct DDFSRCFILE
{
    char fileName[MAX_PATH];
    char cabName[DDF_MAX_CABNAME];
    BOOL extract;
    TCOMP cmpType;
    struct DDFSRCFILE * next;
} DDFSRCFILE;

unsigned int ParseDdf(char * ddfFile, PCCAB vars, DDFSRCFILE ** srcListH, unsigned int v);

#ifdef __cplusplus
}
#endif

#endif /* __PARSEDDF_H__ */
