//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
#include <stdio.h>
#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitInit.h>

// pointers to our instance
static LibreOfficeKit* kit;
static LibreOfficeKitDocument* document;


// Bridge functions to LibreOfficeKit
int LOkit_Init()
{
    // Initialize LibreOfficeKit
    kit = lok_init(NULL);
    return 0;
}


int LOkit_open(char *file)
{
    document = kit->pClass->documentLoad(kit, file);
    document->pClass->initializeForRendering(document, "");
    return 0;
}


#if 0
// createpath from tiled
static NSString *createPaths(NSString *base, NSString *appRootEscaped, NSArray *fileNames)
{
    NSString *result;
    NSString *prefix = @"file://";
    BOOL first = YES;

    result = base;
    for (NSString *fileName in fileNames) {
        result = [result stringByAppendingString: prefix];
        result = [result stringByAppendingString: [appRootEscaped stringByAppendingPathComponent: fileName]];

        if (first) {
            prefix = [@" " stringByAppendingString:prefix];
            first = NO;
        }
    }

    return result;
}


// Force reference to libreofficekit_hook
extern "C" void *libreofficekit_hook(const char *);
static __attribute__((used)) void *(*foop)(const char *) = libreofficekit_hook;

extern "C" void lo_initialize(NSString *documentPath)
{
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString *app_root_escaped = [bundlePath stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    NSString *uno_types = createPaths(@"-env:UNO_TYPES=", app_root_escaped, @[@"offapi.rdb", @"oovbaapi.rdb", @"types.rdb"]);
    NSString *uno_services = createPaths(@"-env:UNO_SERVICES=", app_root_escaped, @[@"ure/services.rdb", @"services.rdb"]);

    int fd = open([[bundlePath stringByAppendingPathComponent:@U_ICUDATA_NAME".dat"] UTF8String], O_RDONLY);
    if (fd != -1) {
        struct stat st;
        if (fstat(fd, &st) != -1
            && st.st_size < (size_t)-1) {
            void *icudata = mmap(0, (size_t) st.st_size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
            if (icudata == MAP_FAILED) {
#if OSL_DEBUG_LEVEL > 0
                NSLog(@"mmap failed:%s", strerror(errno));
#endif
            } else {
                UErrorCode icuStatus = U_ZERO_ERROR;
                udata_setCommonData(icudata, &icuStatus);
#if OSL_DEBUG_LEVEL > 0
                if (U_FAILURE(icuStatus))
                    NSLog(@"udata_setCommonData failed");
                else {
                    // Test that ICU works...
                    UConverter *cnv = ucnv_open("iso-8859-3", &icuStatus);
                    NSLog(@"ucnv_open(iso-8859-3)-> %p, err = %s, name=%s",
                          (void *)cnv, u_errorName(icuStatus), (!cnv)?"?":ucnv_getName(cnv,&icuStatus));
                    if (U_SUCCESS(icuStatus))
                        ucnv_close(cnv);
                }
#endif
            }
        }
        close(fd);
    }

    const char *argv[] = {
        [[[NSBundle mainBundle] executablePath] UTF8String],
        "-env:URE_INTERNAL_LIB_DIR=file:///",
        [uno_types UTF8String],
        [uno_services UTF8String],
        [[@"file://" stringByAppendingString: [documentPath stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]] UTF8String]
    };

    const int argc = sizeof(argv)/sizeof(*argv);

    osl_setCommandArgs(argc, (char **) argv);
}

#endif


#if 0
// LOK_main (online)
#endif


// CLIENT COMMAND INTERFACE
static bool client_canceltiles(const char *args[])
{
    return true;
}

static bool client_clientvisiblearea(const char *args[])
{
    return true;
}

static bool client_closedocument(const char *args[])
{
    return true;
}

static bool client_commandvalues(const char *args[])
{
    return true;
}

static bool client_downloadas(const char *args[])
{
    return true;
}

static bool client_getchildid(const char *args[])
{
    return true;
}

static bool client_gettextselection(const char *args[])
{
    return true;
}

static bool client_insertfile(const char *args[])
{
    return true;
}

static bool client_key(const char *args[])
{
    return true;
}

static bool client_load(const char *args[])
{
    return true;
}

static bool client_loolclient(const char *args[])
{
    return true;
}

static bool client_mouse(const char *args[])
{
    return true;
}

static bool client_paste(const char *args[])
{
    return true;
}

static bool client_ping(const char *args[])
{
    return true;
}

static bool client_renderfont(const char *args[])
{
    return true;
}

static bool client_requestloksession(const char *args[])
{
    return true;
}

static bool client_resetselection(const char *args[])
{
    return true;
}

static bool client_saveas(const char *args[])
{
    return true;
}

static bool client_selectgraphic(const char *args[])
{
    return true;
}

static bool client_selecttext(const char *args[])
{
    return true;
}

static bool client_setclientpart(const char *args[])
{
    return true;
}

static bool client_status(const char *args[])
{
    return true;
}

static bool client_styles(const char *args[])
{
    return true;
}

static bool client_tile(const char *args[])
{
    return true;
}

static bool client_tilecombine(const char *args[])
{
    return true;
}

static bool client_uno(const char *args[])
{
    return true;
}

static bool client_useractive(const char *args[])
{
    return true;
}

static bool client_userinactive(const char *args[])
{
    return true;
}



typedef struct {const char *command; bool useArgs; bool (*funcCmd)(const char *[]);} CLIENTCOMMAND;
static CLIENTCOMMAND clientcommands[] = {{"canceltiles",       false, client_canceltiles},
                                         {"clientvisiblearea", true,  client_clientvisiblearea},
                                         {"closedocument",     false, client_closedocument},
                                         {"commandvalues",     false, client_commandvalues},
                                         {"downloadas",        true,  client_downloadas},
                                         {"getchildid",        false, client_getchildid},
                                         {"gettextselection",  false, client_gettextselection},
                                         {"insertfile",        true,  client_insertfile},
                                         {"key",               true,  client_key},
                                         {"load",              true,  client_load},
                                         {"loolclient",        true,  client_loolclient},
                                         {"mouse",             true,  client_mouse},
                                         {"paste",             true,  client_paste},
                                         {"ping",              false, client_ping},
                                         {"renderfont",        true,  client_renderfont},
                                         {"requestloksession", false, client_requestloksession},
                                         {"resetselection",    false, client_resetselection},
                                         {"saveas",            true,  client_saveas},
                                         {"selectgraphic",     true,  client_selectgraphic},
                                         {"selecttext",        true,  client_selecttext},
                                         {"setclientpart",     true,  client_setclientpart},
                                         {"status",            false, client_status},
                                         {"styles",            false, client_styles},
                                         {"tile",              true,  client_tile},
                                         {"tilecombine",       true,  client_tilecombine},
                                         {"uno",               false, client_uno},
                                         {"useractive",        false, client_useractive},
                                         {"userinactive",      false, client_userinactive},
                                         {NULL,                false, NULL}
                                        };




int LOkit_ClientCommand(const char *input)
{
    static char argStore[2048];
    const char *args[10];
    char *cmd, *arg;
    const char *sep = " ";
    int   j = 0;

    // Split input into cmd + args
    strcpy(argStore, input);
    arg = cmd = strtok(argStore, sep);

    // Locate correct command
    CLIENTCOMMAND *client_cmd = clientcommands;
    for (; client_cmd->command; ++client_cmd)
        if (!strcmp(client_cmd->command,cmd)) {
            args[j++] = input;
            args[j] = NULL;
            if (client_cmd->useArgs)
                while (arg && j < 10)
                {
                    arg = strtok(NULL, sep);
                    args[j++] = arg;
                }
            return (int)client_cmd->funcCmd(args);
        }


    // Call/Return from command
    return -999;
}


