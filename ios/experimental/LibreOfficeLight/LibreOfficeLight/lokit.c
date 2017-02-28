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
static bool client_canceltiles(char *input, char *args[])
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

static bool client_clientvisiblearea(char *input, char *args[])
{
    // command:   clientvisiblearea
    // arguments: x=<x>
    //            y=<y>
    //            width=<width>
    //            height=<height>
    return true;
}

static bool client_closedocument(char *input, char *args[])
{
    // command:   closedocument
    // arguments: None
    // description:
    //     This gives document owners the ability to terminate all sessions
    //     currently having that document opened. This functionality is enabled
    //     only in case WOPI. host mentions 'EnableOwnerTermination' flag in
    //     its CheckFileInfo response
    return true;
}

static bool client_commandvalues(char *input, char *args[])
{
    // command:   commandvalues
    // arguments: None
    return true;
}

static bool client_downloadas(char *input, char *args[])
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
    return true;
}

static bool client_getchildid(char *input, char *args[])
{
    // command:   getchildid
    // arguments: None
    // description:
    //     Requests the child id so that it knows where the files needs to be sent
    //     when it is inserted in the document
    return true;
}

static bool client_gettextselection(char *input, char *args[])
{
    // command:   gettextselection
    // arguments: None
    // description:
    //     Request selection's content
    return true;
}

static bool client_insertfile(char *input, char *args[])
{
    // command:   insertfile
    // arguments: name=<name>
    //            type=<type>
    // description:
    //     Inserts the file with the name <name> into the document,
    //     we currently support type = 'graphic'
    return true;
}

static bool client_key(char *input, char *args[])
{
    // command:   key
    // arguments: type=<type>
    //            char=<charcode>
    //            key=<keycode>
    // description:
    //     <type> is 'input' or 'up', <charcode> and <keycode> are numbers.
    return true;
}

static bool client_load(char *input, char *args[])
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
    return true;
}

static bool client_loolclient(char *input, char *args[])
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
    return true;
}

static bool client_mouse(char *input, char *args[])
{
    // command:   mouse
    // arguments: type=<type>
    //            x=<x>
    //            y=<y>
    //            count=<count>
    // description:
    //     <type> is 'buttondown', 'buttonup' or 'move', others are numbers.
    return true;
}

static bool client_paste(char *input, char *args[])
{
    // command:   paste
    // arguments: mimetype=<mimeType>
    //            <binaryPasteData>
    // description:
    //     Paste content at the current cursor position
    return true;
}

static bool client_ping(char *input, char *args[])
{
    // command:  ping
    // argument: None
    // description:
    //     requests a 'pong' server message.
    return true;
}

static bool client_renderfont(char *input, char *args[])
{
    // command:   renderfont
    // arguments: font=<font>
    //            char=<characters>
    // description:
    //     requests the rendering of the given font.
    //     The font parameter is URL encoded
    //     The char parameter is URL encoded
    return true;
}

static bool client_requestloksession(char *input, char *args[])
{
    // command:   requestloksession
    // arguments: None
    // description:
    //     requests the initialization of a LOK process in an attempt
    //     to predict the user's interaction with the document
    return true;
}

static bool client_resetselection(char *input, char *args[])
{
    // command:   resetselection
    // arguments: None
    return true;
}

static bool client_saveas(char *input, char *args[])
{
    // command:   saveas
    // arguments: url=<url>
    //            format=<format>
    //            options=<options>
    // description:
    //     <url> is a URL, encoded. <format> is also URL-encoded,
    //     i.e. spaces as %20 and it can be empty
    //     options are the whole rest of the line, not URL-encoded, and can be empty
    return true;
}

static bool client_selectgraphic(char *input, char *args[])
{
    // command:   selectgraphic
    // arguments: type=<type>
    //            x=<x>
    //            y=<y>
    // description:
    //     <type> is 'start' or 'end'. <x> and <y> are numbers.
    return true;
}

static bool client_selecttext(char *input, char *args[])
{
    // command:   selecttext
    // arguments: type=<type>
    //            x=<x>
    //            y=<y>
    // description:
    //     <type> is 'start', 'end' or 'reset', <x> and <y> are numbers.
    return true;
}

static bool client_setclientpart(char *input, char *args[])
{
    // command:   setclientpart
    // arguments: part=<partNumber>
    // description:
    //     Informs the server that the client changed to part <partNumber>.
    return true;
}

static bool client_status(char *input, char *args[])
{
    // command:   status
    // arguments: None
    return true;
}

static bool client_styles(char *input, char *args[])
{
    // command:   styles
    // arguments: None
    return true;
}

static bool client_tile(char *input, char *args[])
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
    return true;
}

static bool client_tilecombine(char *input, char *args[])
{
    // command:   tilecombine
    // arguments: <parameters>
    // description:
    //     Accepts same parameters as the 'tile' message except that
    //     parameters 'tileposx', 'tileposy' and 'oldhash' are
    //     comma-separated lists, and the number of elements in each
    //     must be same.
    return true;
}

static bool client_uno(char *input, char *args[])
{
    // command:   uno
    // arguments: <command>
    // description:
    //     <command> is a line of text.
    return true;
}

static bool client_useractive(char *input, char *args[])
{
    // command:   useractive
    // arguments: None
    // description:
    //     Sent when the user regains focus or clicks within the active
    //     area to disable the inactive state.
    //     Will send invalidation and update notifications to force refreshing the screen.
    //     See 'userinactive'.
    return true;
}

static bool client_userinactive(char *input, char *args[])
{
    // command:   userinactive
    // arguments: None
    // description:
    //     Sent when the user has switched tabs or away from the Browser
    //     altogether. It should throttle updates until the user is active again.
    //     See 'useractive'.
    return true;
}



typedef struct {const char *command; bool (*funcCmd)(char *, char *[]);} CLIENTCOMMAND;
static CLIENTCOMMAND clientcommands[] = {{"canceltiles",       client_canceltiles},
                                         {"clientvisiblearea", client_clientvisiblearea},
                                         {"closedocument",     client_closedocument},
                                         {"commandvalues",     client_commandvalues},
                                         {"downloadas",        client_downloadas},
                                         {"getchildid",        client_getchildid},
                                         {"gettextselection",  client_gettextselection},
                                         {"insertfile",        client_insertfile},
                                         {"key",               client_key},
                                         {"load",              client_load},
                                         {"loolclient",        client_loolclient},
                                         {"mouse",             client_mouse},
                                         {"paste",             client_paste},
                                         {"ping",              client_ping},
                                         {"renderfont",        client_renderfont},
                                         {"requestloksession", client_requestloksession},
                                         {"resetselection",    client_resetselection},
                                         {"saveas",            client_saveas},
                                         {"selectgraphic",     client_selectgraphic},
                                         {"selecttext",        client_selecttext},
                                         {"setclientpart",     client_setclientpart},
                                         {"status",            client_status},
                                         {"styles",            client_styles},
                                         {"tile",              client_tile},
                                         {"tilecombine",       client_tilecombine},
                                         {"uno",               client_uno},
                                         {"useractive",        client_useractive},
                                         {"userinactive",      client_userinactive},
                                         {NULL,                NULL}
                                        };




int LOkit_ClientCommand()
{
    char *args[10];
    char *sep = " ";
    char *cmd, *token;
    char *input = "unknown command jan var her";
    int   j = 0;

    // Split input into cmd + args
    token = cmd = strtok(input, sep);
    while (token && j < 10 && token - input < 2048)
      args[j++] = token = strtok(token, sep);

    // Locate correct command
    CLIENTCOMMAND *client_cmd = clientcommands;
    for (; client_cmd->command; ++cmd)
        if (!strcmp(client_cmd->command,"mycommand"))
            return client_cmd->funcCmd(input, NULL);

    // Call/Return from command
    return 0;
}


