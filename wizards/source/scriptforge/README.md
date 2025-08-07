# ScriptForge
## Purpose
Build an extensible and robust collection of macro scripting resources for LibreOffice to be invoked as an **object-oriented API** (Application Programming Interface) from user Basic and/or Python scripts.

## Context
The LibreOffice software is a complete, feature-rich office productivity suite that provides, in combination with an API called "UNO" (Universal Network Objects), all the essential tools for building and deploying custom solutions that build on, or integrate with, the LibreOffice suite of products.

The ScriptForge project proposes to wrap many UNO interfaces in a specific set of classes, methods and properties to promote the development of custom solutions by non-professional developers. Nevertheless, all the UNO functionalities remain available.

## Integration into LibreOffice
The LibreOffice *build process* incorporates the ScriptForge software in each distributed version. It appears to users in source as additional "**LibreOffice Macros & Dialogs**" libraries and their equivalents for Python.
 
Any pre-existing Basic or Python library in the LibreOffice software is left unchanged.

The numbering of the ScriptForge versions is identical with the numbering used by LibreOffice (so far from 7.1 to 26.2).

## Deliverables summary (Version "master")
- A set of Basic + Python modules and classes covering next topics
     - about data containers
          + an extensive set of primitives for array handling, including sorts, set operations and interfaces with csv files and Calc sheets
          + an extensive set of primitives for string handling, including replacements, regular expressions,  encodings, hashing and localization
          + a Dictionary mapping class
     - a coherent error handling for both user and ScriptForge scripts
     - complete FileSystem and TextStream classes for file and directory handling, and text files read & write
     - detailed context information: platform, operating system, special directories, ...
     - interconnection of Basic and Python modules
     - easy access to and management of actual windows  and documents
     - read/write access to data contained in databases
     - management of forms and their controls (Base, Calc, Writer)
     - specific modules for
           + management of multilingual environments
           + automation on Calc sheets
           + static or dynamic, modal or non-modal dialogs and their controls
           + toolbars and their buttons
           + usual menus, context and popup menus
           + automate unit testing of Basic scripts
- The corresponding unit tests grouped in a QA library
- The user documentation as a set of LibreOffice Help pages

## Extensibility
The implemented modules and classes will be invoked from user scripts as **"Services"**. A generic constructor of those services has been designed for that purpose:
```
    CreateScriptService("servicename"[, arg0, arg1, ...])
```
All top-level services are created through this mechanism.

To become a candidate for incorporation into ScriptForge, a library of services must <u>register</u> its services by implementing a `RegisterScriptServices(...)` method. This (simple) method must be present in any external library wanting to extend the set of available services. 
 
Above mechanisms make ScriptForge an easily extensible ecosystem combining a core library, additional standard libraries and libraries published as extensions or distributed as company extensions.

## Actual set of services by library
- ScriptForge
    - Array
    - Dictionary
    - Exception
    - FileSystem
    - L10N
    - Platform
    - Region
    - Session
    - String
    - TextStream
    - Timer
    - UI
- Databases
    - Database
    - Dataset
    - Datasheet
- Dialogs
    - Dialog
    - DialogControl
- Documents
    - Base
    - Calc
    - Chart
    - Document
    - Form
    - FormControl
    - FormDocument
    - Writer
- UnitTests
    - UnitTest
- Widgets
    - ContextMenu
    - Menu
    - PopupMenu
    - Toolbar
    - ToolbarButton

## Portability
*The whole set of services (except when better done by native built-in functions) is made available for Python scripts with identical syntax and behaviour as in Basic.*


## References
- ScriptForge user documentation
<br>[Nightly build](https://help.libreoffice.org/master/en-US/text/sbasic/shared/03/lib_ScriptForge.html?DbPAR=BASIC)
<br>[Last official published version](https://help.libreoffice.org/latest/en-US/text/sbasic/shared/03/lib_ScriptForge.html?DbPAR=BASIC)

- ScriptForge in the TDF wiki
<br>[Macros/ScriptForge](https://wiki.documentfoundation.org/Macros/ScriptForge)

- LibreOffice core repository
<br>[https://cgit.freedesktop.org/libreoffice/core/tree](URL)
<br>[https://cgit.freedesktop.org/libreoffice/core/tree/wizards](URL)
<br>[https://cgit.freedesktop.org/libreoffice/core/tree/wizards/source](URL)

## Change log

### 25.8
  * The new *XRectangle(range)* property (**Calc** service) returns the coordinates (in pixels) on the screen where the given range is located. This opens the door to effective use of popup menus. See [Wiki: popup menu](https://wiki.documentfoundation.org/Macros/ScriptForge/PopupMenuExample).
  * The translation in Spanish is now available.
  * The *Intersect(range1, range2)* method computes the common area of 2 input ranges.
  * With the *RemoveAllItems()* method (**ContextMenu** service), you can opt for configuring a totally customized context menu. Example in [Wiki: context menu](https://wiki.documentfoundation.org/Macros/ScriptForge/CustomContextMenuExample).
  * To avoid duplicates, the *MenuHeaders* property (**Document** service) lists the menus present in the menubar.
  * The **Calc** service now hosts new formatting methods (*AlignRange()*, *BorderRange()*, *ColorizeRange()*, *DecorateFont()* and *FormatRange()*). See an example on [Wiki: format range](https://wiki.documentfoundation.org/Macros/ScriptForge/FormatPortionsCalcRangeExample).
  * The new *CreateTabPageContainer()* method inserts such controls in **dialogs**. Page switching in dialogs: [Wiki: tabbed pages](https://wiki.documentfoundation.org/Macros/ScriptForge/TabbedDialogExample).
  * Still in **dialogs**, the *ImportControl()* method copies controls between dialogs: [Wiki: building compound dialogs](https://wiki.documentfoundation.org/Macros/ScriptForge/BasicMacroOrganizer)
  * The new *UntitledPrefix* property (**Platform** service) provides the prefix used to name new documents. This can make macros more language agnostic.
  * The **Exception** service is revised : the *ReportScriptErrors*, *StopWhenError*, *ReturnCode* and *ReturnCodeDescription* properties let the author of scripts decide what ScriptForge should do when an error is detected by the software.
  * The *CreateBaseDocument()* method (**UI** service) lets you define, in addition to HSQLDB, FIREBIRD and CALC (embedded) database types, also the type *FIREBIRD_EXTERNAL*.
  * The new *DefinedNames* property associated with the *DefineName()* method (**Calc** service) lets you manage defined names both at sheet and global levels.

### 25.2
  * A context menu is usually predefined at LibreOffice installation. Customization is done statically with the Tools + Customize dialog. The (new) **ContextMenu**  service provides a mean to make temporary additions at the bottom of a context menu.
  * When a document is closed inadvertently by the user during a macro run, or between two macros triggered by events, the actual behaviour is a fatal error message. Now the user script may test at any moment if everything is ok with the *IsAlive* As Boolean property applied to next services: **Document**, **Base**, **Calc**, **FormDocument**, **Writer**, **Datasheet**, **Dialog**.
  * To enhance the compatibility between scriptforge dictionaries, python dicts and arrays of PropertyValues, the **Dictionary** service supports now optionally case-sensitive keys, i.e. keys are different if a case-sensitive comparison finds them different.
  * A Python script intended to connect as an outside process to LibreOffice must use either a socket or a pipe connection. So far, only sockets were supported, now both are admitted.
  * One can specify the fore- and background colors of the APSO console opened from a user script with *PythonShell()*.
  * A data array is a 1D array of 1D subarrays. Three methods, *ConvertFromDataArray()*, *ConvertToDataArray()* and *ConvertToRange()* are added to the **Array** service to manage them, especially in the context of complex **Calc** functions.
  * The close button (top-right cross) of a non-modal dialog now closes the dialog as expected.

### 24.8
  * Advanced Python IDE's like PyCharm or VSCode utilize **type hints** to offer autocomplete suggestions during code writing by analyzing type annotations and combining them with the code context. Type hints, also referred to as type annotations, are comments embedded in the code specifying the data types of variables, parameters, and return values. *Python scripts may benefit from the support of type hints when using the ScriptForge API*.

### 24.2
  * The (new) **Dataset** service helps browsing through the data collected with tables, queries or SQL SELECT statements. Getting and updating records data is made easy with the use of dictionaries field name => field value. Binary fields can be extracted to files or updated with the content of files.
  * The **Database** service supports the transaction mode with in particular the *Commit()* and *Rollback()* methods.
  * Use the new *filesystem* property of the **Document** service to navigate through the internal file structure of documents like in usual folders and files.
  * The *Files()* and *Subfolders()* methods in the **FileSystem** service provide the list of files and folders present in a folder. They accept now an additional IncludeSubfolders boolean argument.
  * The *UserData* property of the **Platform** service gives access to the content of the equivalent page in the Tools/Options dialog.
  * The **Documents** service is enriched with a set of functions to manage styles. E.g. suppressing all unused styles can be done in a few lines of code.

### 7.6
  * The (new) **FormDocument** service (a form document is also known as a "Base form", but this is confusing): open (even without first opening the Base document container), close, print, export to PDF, menubar management, access to individual controls.
  * The (new) **Toolbar** and **ToolbarButton** services: hide/show built-in or custom toolbars, hide/show individual toolbar buttons, get or set the script or command to execute when clicked.
  * In the **Calc** service: ranges may be sorted on any number of keys. Also a new *RemoveDuplicates()* method, to clear or to compact ranges, keeping only one copy of identical records.
  * A new *Echo()* method in the **Document** service to freeze screen updates during scripts or to change the actual mouse pointer.
  * Many improvements on the **Dialog** and **DialogControl** services:
    * Support of the Hyperlink control type
    * Dialog controls may be resized. The height and width are expressed in Map AppFont units, like in the Basic IDE.
    * All the *On properties* (to specify the script to be executed when an event occurs) are now editable.
    * Dialog controls may be created dynamically.
    * Dialog controls may be cloned with the new *CloneControl()* method.
    * A dialog can be created from scratch.
    * Tabulations between controls are defined at once by the new *OrderTabs()* method.

### 7.5
  * The (new) **Datasheet** service, to manage tabular views of database data. A datasheet may be opened from a Base window or at any moment (without the Base component being open). Cursor movement and current selection content may be processed by a user script.
  * The *RunCommand()* method may receive arguments (see [.UNO:Commands wiki page](https://wiki.documentfoundation.org/Development/DispatchCommands)).
  * Error messages provide direct access to the help page of the failing method.
  * A new *Normalize()* method in the **FileSystem** service.
  * In the   *  service, new uses of the FilterFormula concept to selectively clear or format cells, rows or columns.
  * Support of 16,384 columns in the **Calc** service.
  * In the **Dialog** service, a new *SetPageManager()* method synchronizes automatically dropdown lists, next/previous buttons and tab headers while the user browses through the dialog pages.
  * A new **basic**.*CreateUnoStruct()* method for Python scripts.

### 7.4
  * The (new) **Menu** service, to describe a new menu to be displayed in the menubar. The menu may contain usual items, checkboxes or radio buttons. Each item can be decorated with icons and tooltips.
  * The (new) **UnitTest** service, a framework to setup testcases and testsuites for complex Basic modules and extensions.
  * The (new) **Region** service , to manage locales, timezones and daylight saving times. What time is it in Tokyo when it is 12am in Brussels ? Also to convert numbers to letters in any language.
  * Get the list of the available extensions and their install directories (**FileSystem** service).
  * Run menu commands with arguments (**UI**, **Document** services).object-oriented API
  * Manage a fallback language (**L10N** service).
  * Export sheet ranges as images or PDF files (**Calc** service).
  * Erase rows or columns from sheet ranges based on filters expressed as formulas (default = erase all empty rows or columns) (**Calc** service).
  * Create and place a pivot table (**Calc** service).
  * Resize and/or center dialogs on a background window or dialog (**Dialog** service).

### 7.3
  * The (new) **Chart** service, to define chart documents stored in Calc sheets. Most parameters available in the user interface can be set by script.
  * The (new) **PopupMenu** service, to describe the menu to be displayed after a mouse event (typically a right-click). The menu may contain usual items, checkboxes or radio buttons. Each item can be decorated with icons and tooltips.
  * Management of printers: list of fonts and printers, printer options, printing documents.
  * Export of documents to PDF and management of PDF options.
  * **Dialogs** may host table controls. Provide tabular data, the data is displayed with headers and sort buttons. The selected row is returned by the Value property.
  * Titles, tooltips, list- or comboboxes, labels of dialogs can be translated on-the-fly based on tools provided in the **L10N** service.
  * The **Calc** service has been improved with facilities to identify cell ranges when their size is unknown, f.i. after the import of data, and to apply formulas with absolute and relative references on them.

### 7.2 (BASIC + PYTHON)
  * The **DialogControl** service supports tree controls including the OnNodeSelected and OnNodeExpanded events.
  * The document management part is enriched with the new **Form** and **FormControl** services. A form and its subforms may be located in a Base, Writer or Calc document.
  * *The whole set of services (except for those functions that are better handled by Python natively) is made available for Python scripts with identical syntax and behaviour as in Basic.*
  * In addition, a set of methods is provided compatible with their homonymous **Basic** builtin functions (MsgBox, CreateUnoService, ...). ScriptForge also integrates the APSO shell console, providing the previous installation of the [APSO extension](https://extensions.libreoffice.org/en/extensions/show/apso-alternative-script-organizer-for-Python).

### 7.1 INITIAL RELEASE (BASIC ONLY)
  * About data containers:
    * an extensive set of primitives for **array** handling, including sorts, set operations and interfaces with csv files and Calc sheets
    * an extensive set of primitives for **string** handling, including replacements, regular expressions, encodings, hashing and localization
    * a **Dictionary** mapping class
  * A coherent error handling for both user and ScriptForge scripts
  * Complete **FileSystem** and **TextStream** classes for file and directory handling, and text files read & write
  * Detailed context information: platform, operating system, directories, ...
  * Interconnection of Basic and Python modules
  * Easy access to and management of actual windows and documents
  * Specific modules for:
    * automation on **Calc** sheets
    * management of dialogs and their controls
    * access to data contained in **databases**

