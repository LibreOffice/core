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
    // command:   canceltiles
    // arguments: NONE
    // description:
    //     All outstanding tile messages from the client to the server are
    //     dropped and will not be handled, except tile messages with an id
    //     parameter. There is no guarantee of exactly which tile: messages
    //     might still be sent back to the client.
    //
    return true;
}

static bool client_clientvisiblearea(const char *args[])
{
    // command:   clientvisiblearea
    // arguments: x=<x>
    //            y=<y>
    //            width=<width>
    //            height=<height>

    // JIX MISSING CODE
    return true;
}

static bool client_closedocument(const char *args[])
{
    // command:   closedocument
    // arguments: None
    // description:
    //     This gives document owners the ability to terminate all sessions
    //     currently having that document opened. This functionality is enabled
    //     only in case WOPI. host mentions 'EnableOwnerTermination' flag in
    //     its CheckFileInfo response

    // JIX MISSING CODE
    return true;
}

static bool client_commandvalues(const char *args[])
{
    // command:   commandvalues
    // arguments: None

    // JIX MISSING CODE
    return true;
}

static bool client_downloadas(const char *args[])
{
    // command:   downloadas
    // arguments: name=<fileName>
    //            id=<id>
    //            format=<document format>
    //            options=<SkipImages, etc>
    // description:
    //     Exports the current document to the desired format and returns a
    //     download URL. The id identifies the request on the client. id can
    //     take following values:
    //         * 'print': When request for download is basically for print purposes
    //         * 'slideshow': When request for download is for showing slideshow
    //         * 'export': Just a simple download

    // JIX MISSING CODE
    return true;
}

static bool client_getchildid(const char *args[])
{
    // command:   getchildid
    // arguments: None
    // description:
    //     Requests the child id so that it knows where the files needs to be sent
    //     when it is inserted in the document
    return true;
}

static bool client_gettextselection(const char *args[])
{
    // command:   gettextselection
    // arguments: None
    // description:
    //     Request selection's content

    // JIX MISSING CODE
    return true;
}

static bool client_insertfile(const char *args[])
{
    // command:   insertfile
    // arguments: name=<name>
    //            type=<type>
    // description:
    //     Inserts the file with the name <name> into the document,
    //     we currently support type = 'graphic'

    // JIX MISSING CODE
    return true;
}

static bool client_key(const char *args[])
{
    // command:   key
    // arguments: type=<type>
    //            char=<charcode>
    //            key=<keycode>
    // description:
    //     <type> is 'input' or 'up', <charcode> and <keycode> are numbers.

    // JIX MISSING CODE
    return true;
}

static bool client_load(const char *args[])
{
    // command:   load
    // arguments: [part=<partNumber>]
    //             url=<url>
    //            [timestamp=<time>]
    //.           [options=<options>]
    // description:
    //     part is an optional parameter. <partNumber> is a number.
    //     timestamp is an optional parameter. <time> is provided in
    //     microseconds since the Unix epoch - midnight, January 1, 1970.
    //     options are the whole rest of the line, not URL-encoded,
    //     and must be valid JSON.

    // JIX MISSING CODE
    return true;
}

static bool client_loolclient(const char *args[])
{
    // command:   loolclient
    // arguments: <major.minor[-patch]>
    // description:
    //     Upon connection, a client must announce the version number it supports.
    //     Major: an integer that must always match between client and server,
    //     otherwise there are no guarantees of any sensible compatibility.
    //     This is bumped when API changes.
    //     Minor: an integer is more flexible and is at the discretion of either party.
    //     Security fixes that do not alter the API would bump the minor version number.
    //     Patch: an optional string that is informational.

    // JIX MISSING CODE
    return true;
}

static bool client_mouse(const char *args[])
{
    // command:   mouse
    // arguments: type=<type>
    //            x=<x>
    //            y=<y>
    //            count=<count>
    // description:
    //     <type> is 'buttondown', 'buttonup' or 'move', others are numbers.

    // JIX MISSING CODE
    return true;
}

static bool client_paste(const char *args[])
{
    // command:   paste
    // arguments: mimetype=<mimeType>
    //            <binaryPasteData>
    // description:
    //     Paste content at the current cursor position

    // JIX MISSING CODE
    return true;
}

static bool client_ping(const char *args[])
{
    // command:  ping
    // argument: None
    // description:
    //     requests a 'pong' server message.

    // JIX MISSING CODE
    return true;
}

static bool client_renderfont(const char *args[])
{
    // command:   renderfont
    // arguments: font=<font>
    //            char=<characters>
    // description:
    //     requests the rendering of the given font.
    //     The font parameter is URL encoded
    //     The char parameter is URL encoded

    // JIX MISSING CODE
    return true;
}

static bool client_requestloksession(const char *args[])
{
    // command:   requestloksession
    // arguments: None
    // description:
    //     requests the initialization of a LOK process in an attempt
    //     to predict the user's interaction with the document

    // JIX MISSING CODE
    return true;
}

static bool client_resetselection(const char *args[])
{
    // command:   resetselection
    // arguments: None

    // JIX MISSING CODE
    return true;
}

static bool client_saveas(const char *args[])
{
    // command:   saveas
    // arguments: url=<url>
    //            format=<format>
    //            options=<options>
    // description:
    //     <url> is a URL, encoded. <format> is also URL-encoded,
    //     i.e. spaces as %20 and it can be empty
    //     options are the whole rest of the line, not URL-encoded, and can be empty

    // JIX MISSING CODE
    return true;
}

static bool client_selectgraphic(const char *args[])
{
    // command:   selectgraphic
    // arguments: type=<type>
    //            x=<x>
    //            y=<y>
    // description:
    //     <type> is 'start' or 'end'. <x> and <y> are numbers.

    // JIX MISSING CODE
    return true;
}

static bool client_selecttext(const char *args[])
{
    // command:   selecttext
    // arguments: type=<type>
    //            x=<x>
    //            y=<y>
    // description:
    //     <type> is 'start', 'end' or 'reset', <x> and <y> are numbers.

    // JIX MISSING CODE
    return true;
}

static bool client_setclientpart(const char *args[])
{
    // command:   setclientpart
    // arguments: part=<partNumber>
    // description:
    //     Informs the server that the client changed to part <partNumber>.

    // JIX MISSING CODE
    return true;
}

static bool client_status(const char *args[])
{
    // command:   status
    // arguments: None

    // JIX MISSING CODE
    return true;
}

static bool client_styles(const char *args[])
{
    // command:   styles
    // arguments: None

    // JIX MISSING CODE
    return true;
}

static bool client_tile(const char *args[])
{
    // command:   tile
    // arguments: part=<partNumber>
    //            width=<width>
    //            height=<height>
    //            tileposx=<xpos>
    //            tileposy=<ypos>
    //            tilewidth=<tileWidth>
    //            tileheight=<tileHeight>
    //           [timestamp=<time>]
    //           [id=<id>]
    //           [broadcast=<yesOrNo>]
    //           [oldhash=<hash>]
    // description:
    //     Parameters are numbers except broadcast which is 'yes' or 'no'
    //     and hash which is a 64-bit hash. (There is no need for the client
    //     to parse it into a number, it can be treated as an opaque string.)
    //     Note: id must be echoed back in the response verbatim. It and the
    //     following parameter, broadcast, are used when rendering slide
    //     previews of presentation documents, and not for anything else. It
    //     is only useful to loleaflet and will break it if not returned in
    //     the response.

    // JIX MISSING CODE

    // renderTile
    return true;
}

static bool client_tilecombine(const char *args[])
{
    // command:   tilecombine
    // arguments: <parameters>
    // description:
    //     Accepts same parameters as the 'tile' message except that
    //     parameters 'tileposx', 'tileposy' and 'oldhash' are
    //     comma-separated lists, and the number of elements in each
    //     must be same.

    // JIX MISSING CODE

    // renderCombinedTiles
    return true;
}

static bool client_uno(const char *args[])
{
    // command:   uno
    // arguments: <command>
    // description:
    //     <command> is a line of text.

    // JIX MISSING CODE
    return true;
}

static bool client_useractive(const char *args[])
{
    // command:   useractive
    // arguments: None
    // description:
    //     Sent when the user regains focus or clicks within the active
    //     area to disable the inactive state.
    //     Will send invalidation and update notifications to force refreshing the screen.
    //     See 'userinactive'.

    // JIX MISSING CODE
    return true;
}

static bool client_userinactive(const char *args[])
{
    // command:   userinactive
    // arguments: None
    // description:
    //     Sent when the user has switched tabs or away from the Browser
    //     altogether. It should throttle updates until the user is active again.
    //     See 'useractive'.

    // JIX MISSING CODE
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


