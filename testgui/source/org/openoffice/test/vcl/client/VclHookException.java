/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.client;

import java.util.Properties;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * The exception during executing the remote method.
 *
 */
public class VclHookException extends RuntimeException {
    /**
     *
     */
    private static final long serialVersionUID = 1L;

    /**
     * The messages are from basic\source\app\svtmsg.src
     */
    /***ERROR****/
    public static final int SVT_START = 22000;

    public static final int S_GPF_ABORT                                 =( SVT_START +   0 );
    public static final int S_APP_SHUTDOWN                              =( SVT_START +   1 );
    public static final int S_SID_EXECUTE_FAILED_NO_DISPATCHER          =( SVT_START +   2 );
    public static final int S_SID_EXECUTE_FAILED                        =( SVT_START +   3 );
    public static final int S_UNO_PROPERTY_NITIALIZE_FAILED             =( SVT_START +   4 );
    public static final int S_RESETAPPLICATION_FAILED_COMPLEX           =( SVT_START +   5 );
    public static final int S_RESETAPPLICATION_FAILED_UNKNOWN           =( SVT_START +   6 );
    public static final int S_NO_ACTIVE_WINDOW                          =( SVT_START +   7 );
    public static final int S_NO_DIALOG_IN_GETACTIVE                    =( SVT_START +   8 );
    public static final int S_NO_POPUP                                  =( SVT_START +   9 );
    public static final int S_NO_SUBMENU                                =( SVT_START +  10 );
    public static final int S_CONTROLTYPE_NOT_SUPPORTED                 =( SVT_START +  11 );
    public static final int S_SELECTION_BY_ATTRIBUTE_ONLY_DIRECTORIES   =( SVT_START +  12 );
    public static final int S_NO_MORE_FILES                             =( SVT_START +  13 );
    public static final int S_UNKNOWN_METHOD                            =( SVT_START +  14 );
    public static final int S_INVALID_PARAMETERS                        =( SVT_START +  15 );
    public static final int S_POINTER_OUTSIDE_APPWIN                    =( SVT_START +  16 );
    public static final int S_UNKNOWN_COMMAND                           =( SVT_START +  17 );
    public static final int S_WIN_NOT_FOUND                             =( SVT_START +  18 );
    public static final int S_WIN_INVISIBLE                             =( SVT_START +  19 );
    public static final int S_WIN_DISABLED                              =( SVT_START +  20 );
    public static final int S_NUMBER_TOO_BIG                            =( SVT_START +  21 );
    public static final int S_NUMBER_TOO_SMALL                          =( SVT_START +  22 );
    public static final int S_WINDOW_DISAPPEARED                        =( SVT_START +  23 );
    public static final int S_ERROR_SAVING_IMAGE                        =( SVT_START +  24 );
    public static final int S_INVALID_POSITION                          =( SVT_START +  25 );
    public static final int S_SPLITWIN_NOT_FOUND                        =( SVT_START +  26 );
    public static final int S_INTERNAL_ERROR                            =( SVT_START +  27 );
    public static final int S_NO_STATUSBAR                              =( SVT_START +  28 );
    public static final int S_ITEMS_INVISIBLE                           =( SVT_START +  29 );
    public static final int S_TABPAGE_NOT_FOUND                         =( SVT_START +  30 );
    public static final int S_TRISTATE_NOT_ALLOWED                      =( SVT_START +  31 );
    public static final int S_ERROR_IN_SET_TEXT                         =( SVT_START +  32 );
    public static final int S_ATTEMPT_TO_WRITE_READONLY                 =( SVT_START +  33 );
    public static final int S_NO_SELECT_FALSE                           =( SVT_START +  34 );
    public static final int S_ENTRY_NOT_FOUND                           =( SVT_START +  35 );
    public static final int S_METHOD_FAILED                             =( SVT_START +  36 );
    public static final int S_HELPID_ON_TOOLBOX_NOT_FOUND               =( SVT_START +  37 );
    public static final int S_BUTTON_DISABLED_ON_TOOLBOX                =( SVT_START +  38 );
    public static final int S_BUTTON_HIDDEN_ON_TOOLBOX                  =( SVT_START +  39 );
    public static final int S_CANNOT_MAKE_BUTTON_VISIBLE_IN_TOOLBOX     =( SVT_START +  40 );
    public static final int S_TEAROFF_FAILED                            =( SVT_START +  41 );
    public static final int S_NO_SELECTED_ENTRY_DEPRECATED              =( SVT_START +  42 ); // Has to stay in for old res files
    public static final int S_SELECT_DESELECT_VIA_STRING_NOT_IMPLEMENTED =( SVT_START +  43 );
    public static final int S_ALLOWED_ONLY_IN_FLOATING_MODE             =( SVT_START +  44 );
    public static final int S_ALLOWED_ONLY_IN_DOCKING_MODE              =( SVT_START +  45 );
    public static final int S_SIZE_NOT_CHANGEABLE                       =( SVT_START +  46 );
    public static final int S_NO_OK_BUTTON                              =( SVT_START +  47 );
    public static final int S_NO_CANCEL_BUTTON                          =( SVT_START +  48 );
    public static final int S_NO_YES_BUTTON                             =( SVT_START +  49 );
    public static final int S_NO_NO_BUTTON                              =( SVT_START +  50 );
    public static final int S_NO_RETRY_BUTTON                           =( SVT_START +  51 );
    public static final int S_NO_HELP_BUTTON                            =( SVT_START +  52 );
    public static final int S_NO_DEFAULT_BUTTON                         =( SVT_START +  53 );
    public static final int S_BUTTON_ID_NOT_THERE                       =( SVT_START +  54 );
    public static final int S_BUTTONID_REQUIRED                         =( SVT_START +  55 );
    public static final int S_UNKNOWN_TYPE                              =( SVT_START +  56 );
    public static final int S_UNPACKING_STORAGE_FAILED                  =( SVT_START +  57 );
    public static final int S_NO_LIST_BOX_BUTTON                        =( SVT_START +  58 );
    public static final int S_UNO_URL_EXECUTE_FAILED_NO_DISPATCHER      =( SVT_START +  59 );
    public static final int S_UNO_URL_EXECUTE_FAILED_NO_FRAME           =( SVT_START +  60 );
    public static final int S_NO_MENU                                   =( SVT_START +  61 );
    public static final int S_NO_SELECTED_ENTRY                         =( SVT_START +  62 );
    public static final int S_UNO_URL_EXECUTE_FAILED_DISABLED           =( SVT_START +  63 );
    public static final int S_NO_SCROLLBAR                              =( SVT_START +  64 );
    public static final int S_NO_SAX_PARSER                             =( SVT_START +  65 );
    public static final int S_CANNOT_CREATE_DIRECTORY                   =( SVT_START +  66 );
    public static final int S_DIRECTORY_NOT_EMPTY                       =( SVT_START +  67 );
    public static final int S_DEPRECATED                                =( SVT_START +  68 );
    public static final int S_SIZE_BELOW_MINIMUM                        =( SVT_START +  69 );
    public static final int S_CANNOT_FIND_FLOATING_WIN                  =( SVT_START +  70 );
    public static final int S_NO_LIST_BOX_STRING                        =( SVT_START +  71 );
    public static final int S_SLOT_IN_EXECUTE                           =( SVT_START +  72 );

    public static Properties MESSAGES = new Properties();

    static {
        MESSAGES.put(new Integer(S_GPF_ABORT), "Program aborted with GPF");
        MESSAGES.put(new Integer(S_APP_SHUTDOWN), "Application has been shut down");
        MESSAGES.put(new Integer(S_SID_EXECUTE_FAILED_NO_DISPATCHER), "Slot ID cannot be executed. No ActiveDispatcher");
        MESSAGES.put(new Integer(S_SID_EXECUTE_FAILED), "Slot ID could not be executed");
        MESSAGES.put(new Integer(S_UNO_PROPERTY_NITIALIZE_FAILED), "UnoSlot: Properties could not be initialized");
        MESSAGES.put(new Integer(S_RESETAPPLICATION_FAILED_COMPLEX),
                "ResetApplication failed: too complex");
        MESSAGES.put(new Integer(S_RESETAPPLICATION_FAILED_UNKNOWN),
                "ResetApplication failed: unknown window type");
        MESSAGES.put(new Integer(S_NO_ACTIVE_WINDOW),
                "No active window found (GetNextCloseWindow)");
        MESSAGES.put(new Integer(S_NO_DIALOG_IN_GETACTIVE),
                "GetActive does not return a dialog! Inform development");
        MESSAGES.put(new Integer(S_NO_POPUP), "Pop-up menu not open");
        MESSAGES.put(new Integer(S_NO_SUBMENU), "Submenu does not exist");
        MESSAGES.put(new Integer(S_CONTROLTYPE_NOT_SUPPORTED),
                "ControlType ($Arg1) is not supported");
        MESSAGES.put(new Integer(S_SELECTION_BY_ATTRIBUTE_ONLY_DIRECTORIES),
                "Selection by attributes only possible for directories");
        MESSAGES.put(new Integer(S_NO_MORE_FILES), "No more files");
        MESSAGES.put(new Integer(S_UNKNOWN_METHOD),
                "Unknown method '($Arg1)' on ($Arg2)");
        MESSAGES.put(new Integer(S_INVALID_PARAMETERS), "Invalid Parameters");
        MESSAGES.put(new Integer(S_POINTER_OUTSIDE_APPWIN),
                "Pointer not inside application window at '($Arg1)'");
        MESSAGES.put(new Integer(S_UNKNOWN_COMMAND),
                "Unknown command '($Arg1)'");
        MESSAGES.put(new Integer(S_WIN_NOT_FOUND), "($Arg1) could not be found");
        MESSAGES.put(new Integer(S_WIN_INVISIBLE), "($Arg1) is not visible");
        MESSAGES.put(new Integer(S_WIN_DISABLED), "($Arg1) could not be accessed. Disabled");
        MESSAGES.put(new Integer(S_NUMBER_TOO_BIG),
                        "Entry number ($Arg2) is too large in ($Arg1). Max. allowed is ($Arg3)");
        MESSAGES.put(new Integer(S_NUMBER_TOO_SMALL),
                        "The entry number ($Arg2) is too small in ($Arg1). Min allowed is ($Arg3)");
        MESSAGES.put(new Integer(S_WINDOW_DISAPPEARED), "Window disappeared in the meantime at ($Arg1)");
        MESSAGES.put(new Integer(S_ERROR_SAVING_IMAGE), "Error #($Arg1) when saving the image");
        MESSAGES.put(new Integer(S_INVALID_POSITION),
                "Invalid position at ($Arg1)");
        MESSAGES.put(new Integer(S_SPLITWIN_NOT_FOUND),
                "SplitWindow not found at ($Arg1)");
        MESSAGES
                .put(new Integer(S_INTERNAL_ERROR), "Internal error at ($Arg1)");
        MESSAGES.put(new Integer(S_NO_STATUSBAR), "No status bar at ($Arg1)");
        MESSAGES.put(new Integer(S_ITEMS_INVISIBLE),
                "The items are hidden at ($Arg1)");
        MESSAGES.put(new Integer(S_TABPAGE_NOT_FOUND),
                "Tab page not found at ($Arg1)");
        MESSAGES.put(new Integer(S_TRISTATE_NOT_ALLOWED),
                "Tristate cannot be set at ($Arg1)");
        MESSAGES.put(new Integer(S_ERROR_IN_SET_TEXT),
                "Set text did not function");
        MESSAGES.put(new Integer(S_ATTEMPT_TO_WRITE_READONLY),
                "Attempt to write on read-only ($Arg1)");
        MESSAGES.put(new Integer(S_NO_SELECT_FALSE),
                "Select FALSE not allowed. Use MultiSelect at ($Arg1)");
        MESSAGES.put(new Integer(S_ENTRY_NOT_FOUND),
                "\"($Arg2)\" entry at ($Arg1) not found");
        MESSAGES.put(new Integer(S_METHOD_FAILED),
                "($Arg1) of entry \"($Arg2)\" failed");
        MESSAGES.put(new Integer(S_HELPID_ON_TOOLBOX_NOT_FOUND),
                "HelpID in ToolBox not found at ($Arg1)");

        MESSAGES.put(new Integer(S_BUTTON_DISABLED_ON_TOOLBOX),
                "The button is disabled in ToolBox at ($Arg1)");

        MESSAGES.put(new Integer(S_BUTTON_HIDDEN_ON_TOOLBOX),
                "The button is hidden in ToolBox at ($Arg1)");

        MESSAGES.put(new Integer(S_CANNOT_MAKE_BUTTON_VISIBLE_IN_TOOLBOX),
                "Button cannot be made visible in ToolBox at ($Arg1)");

        MESSAGES.put(new Integer(S_TEAROFF_FAILED),
                "TearOff failed in ToolBox at ($Arg1)");

        // Has to stay in for old res files
        MESSAGES.put(new Integer(S_NO_SELECTED_ENTRY_DEPRECATED),
                "No entry is selected in TreeListBox at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_SELECTED_ENTRY),
                "No entry is selected in ($Arg2) at ($Arg1)");

        MESSAGES
                .put(new Integer(S_SELECT_DESELECT_VIA_STRING_NOT_IMPLEMENTED),
                        "Select/Deselect with MESSAGES.put(new Integer(not implemented at ($Arg1)");

        MESSAGES.put(new Integer(S_ALLOWED_ONLY_IN_FLOATING_MODE),
                "Method only allowed in floating mode at ($Arg1)");

        MESSAGES.put(new Integer(S_ALLOWED_ONLY_IN_DOCKING_MODE),
                "Method only allowed in docking mode at ($Arg1)");

        MESSAGES.put(new Integer(S_SIZE_NOT_CHANGEABLE),
                "Size cannot be altered at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_OK_BUTTON),
                "There is no OK button at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_CANCEL_BUTTON),
                "There is no Cancel button at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_YES_BUTTON),
                "There is no Yes button at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_NO_BUTTON),
                "There is no No button at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_RETRY_BUTTON),
                "There is no Repeat button at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_HELP_BUTTON),
                "There is no Help button at ($Arg1)");

        MESSAGES.put(new Integer(S_NO_DEFAULT_BUTTON),
                "There is no Default button defined at ($Arg1)");

        MESSAGES.put(new Integer(S_BUTTON_ID_NOT_THERE),
                "There is no button with ID ($Arg1) at ($Arg2)");

        MESSAGES.put(new Integer(S_BUTTONID_REQUIRED),
                "A button ID needs to be given at ($Arg1)");

        MESSAGES
                .put(new Integer(S_UNKNOWN_TYPE),
                        "Unknown object type ($Arg1) from UId or method '($Arg2)' not supported");

        MESSAGES.put(new Integer(S_UNPACKING_STORAGE_FAILED),
                "Unpacking storage \"($Arg1)\" to \"($Arg2)\" failed");

        MESSAGES.put(new Integer(S_NO_LIST_BOX_BUTTON),
                "ListBoxButton does not exist in ($Arg1)");

        MESSAGES
                .put(new Integer(S_UNO_URL_EXECUTE_FAILED_NO_DISPATCHER),
                        "UNO URL \"($Arg1)\" could not be executed: No dispatcher was found.");

        MESSAGES
                .put(new Integer(S_UNO_URL_EXECUTE_FAILED_NO_FRAME),
                        "UNO URL \"($Arg1)\" could not be executed: No ActiveFrame on desktop.");

        MESSAGES.put(new Integer(S_NO_MENU), "There is no menu at ($Arg1)");

        MESSAGES.put(new Integer(S_UNO_URL_EXECUTE_FAILED_DISABLED),
                "UNO URL \"($Arg1)\" could not be run: Disabled");

        MESSAGES.put(new Integer(S_NO_SCROLLBAR), "No scroll bar at ($Arg1)");

        MESSAGES
                .put(new Integer(S_NO_SAX_PARSER),
                        "No SAX Parser when using ($Arg1). Initialize with 'SAXReadFile' first.");

        MESSAGES.put(new Integer(S_CANNOT_CREATE_DIRECTORY),
                "Cannot create Directory: \"($Arg1)\"");

        MESSAGES
                .put(new Integer(S_DIRECTORY_NOT_EMPTY),
                        "Directory has to be Empty to unpack to. Directory: \"($Arg1)\"");

        MESSAGES.put(new Integer(S_DEPRECATED),
                "Deprecated! Please change the script.");

        MESSAGES.put(new Integer(S_SIZE_BELOW_MINIMUM),
                "The Size is below the minimum. x=($Arg1) ,y=($Arg2)");

        MESSAGES
                .put(new Integer(S_CANNOT_FIND_FLOATING_WIN),
                        "Cannot find FloatingWindow for floating DockingWindow at ($Arg1).");

        MESSAGES.put(new Integer(S_NO_LIST_BOX_STRING),
                "String does not exist in ($Arg1)");

        MESSAGES.put(new Integer(S_SLOT_IN_EXECUTE),
                "Another Slot is being executed already.");

    }

    private SmartId id = null;

    private String message = null;

    private int code = -1;

    private Properties properties = new Properties();

    public VclHookException(String message) {
        this(null, message);
    }

    public VclHookException(SmartId id, String message) {
        this.id = id;
        this.message = message;
        if (id != null)
            parse();
    }


    public String getMessage() {
        return this.message;
    }


    private void parse() {
        if (message == null)
            return;

        //Replace some key
        message = message.replaceAll("%Method=([^%]*)%", "$1");
        message = message.replaceAll("%RType=([^%]*)%", "$1");
        message = message.replaceAll("%RCommand=([^%]*)%", "$1");
        message = message.replaceAll("%UId=([^%]*)%", "$1");

        // Parse String into Properties
        int start = -1, sep = -1, end =-1, pos = 0;
        String key = null;
        String value= null;
        while( (start = message.indexOf('%', pos)) != -1
                && (sep = message.indexOf('=', start + 1)) != -1
                && (end = message.indexOf('%', sep + 1)) != -1) {
            key = message.substring(start + 1, sep);
            value = message.substring(sep + 1, end);
            pos = end + 1;
            properties.put(key, value);
        }

        String resId = properties.getProperty("ResId");
        if (resId == null)
            return;
        this.code = Integer.parseInt(resId);
        String originalMsg = (String) MESSAGES.get(this.code);
        if (originalMsg == null)
            return;

        Pattern pattern = Pattern.compile("\\(\\$([^\\)]*)\\)");
        Matcher matcher = pattern.matcher(originalMsg);
        StringBuffer result = new StringBuffer();
        while (matcher.find()) {
            String rep = properties.getProperty(matcher.group(1), matcher.group()).replace("$", "\\$");
            matcher.appendReplacement(result, rep);
        }
        matcher.appendTail(result);
        message =  "ID:" + id + " - " + result.toString();
    }


    public int getCode() {
        return this.code;
    }
}
