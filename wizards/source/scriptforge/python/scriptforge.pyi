# -*- coding: utf-8 -*-

#     Copyright 2020-2024 Jean-Pierre LEDURE, Rafael LIMA, @AmourSpirit, Alain ROMEDENNE

# =====================================================================================================================
# ===           The ScriptForge library and its associated libraries are part of the LibreOffice project.           ===
# ===                   Full documentation is available on https://help.libreoffice.org/                            ===
# =====================================================================================================================

# ScriptForge is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# ScriptForge is free software; you can redistribute it and/or modify it under the terms of either (at your option):

# 1) The Mozilla Public License, v. 2.0. If a copy of the MPL was not
# distributed with this file, you can obtain one at http://mozilla.org/MPL/2.0/ .

# 2) The GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version. If a copy of the LGPL was not
# distributed with this file, see http://www.gnu.org/licenses/ .

"""
    ScriptForge libraries are an extensible and robust collection of macro scripting resources for LibreOffice
    to be invoked from user Basic or Python macros. Users familiar with other BASIC macro variants often face hard
    times to dig into the extensive LibreOffice Application Programming Interface even for the simplest operations.
    By collecting most-demanded document operations in a set of easy to use, easy to read routines, users can now
    program document macros with much less hassle and get quicker results.

    The use of the ScriptForge interfaces in user scripts hides the complexity of the usual UNO interfaces.
    However, it does not replace them. At the opposite their coexistence is ensured.
    Indeed, ScriptForge provides a number of shortcuts to key UNO objects.

    The scriptforge.py module
        - describes the interfaces (classes and attributes) to be used in Python user scripts
          to run the services implemented in the standard modules shipped with LibreOffice
        - implements a protocol between those interfaces and, when appropriate, the corresponding ScriptForge
          Basic libraries implementing the requested services.

    The scriptforge.pyi module
        - provides the static type checking of all the visible interfaces of the ScriptForge API.
        - when the user uses an IDE like PyCharm or VSCode, (s)he might benefit from the typing
          hints provided by them thanks to the actual scriptforge.pyi module.

    Specific documentation about the use of ScriptForge from Python scripts:
        https://help.libreoffice.org/latest/en-US/text/sbasic/shared/03/sf_intro.html?DbPAR=BASIC
    """
# IMPORTS
from __future__ import annotations
import datetime
import time
from typing import Any, Dict, List, Literal, NewType, Optional, overload, Sequence, Tuple, TypeVar, Union

# List the available service types
#   SFScriptForge
ARRAY = SFScriptForge.SF_Array
BASIC = SFScriptForge.SF_Basic
DICTIONARY = SFScriptForge.SF_Dictionary
EXCEPTION = SFScriptForge.SF_Exception
FILESYSTEM = SFScriptForge.SF_FileSystem
L10N = SFScriptForge.SF_L10N
PLATFORM = SFScriptForge.SF_Platform
REGION = SFScriptForge.SF_Region
SESSION = SFScriptForge.SF_Session
STRING = SFScriptForge.SF_String
TEXTSTREAM = SFScriptForge.SF_TextStream
TIMER = SFScriptForge.SF_Timer
UI = SFScriptForge.SF_UI
#   SFDatabases
DATABASE = SFDatabases.SF_Database
DATASET = SFDatabases.SF_Dataset
DATASHEET = SFDatabases.SF_Datasheet
#   SFDialogs
DIALOG = SFDialogs.SF_Dialog
DIALOGCONTROL = SFDialogs.SF_DialogControl
#   SFDocuments
DOCUMENT = SFDocuments.SF_Document
BASE = SFDocuments.SF_Base
CALC = SFDocuments.SF_Calc
CALCREFERENCE = SFDocuments.SF_CalcReference
CHART = SFDocuments.SF_Chart
FORM = SFDocuments.SF_Form
FORMCONTROL = SFDocuments.SF_FormControl
FORMDOCUMENT = SFDocuments.SF_FormDocument
WRITER = SFDocuments.SF_Writer
#   SFWidgets
MENU = SFWidgets.SF_Menu
POPUPMENU = SFWidgets.SF_PopupMenu
TOOLBAR = SFWidgets.SF_Toolbar
TOOLBARBUTTON = SFWidgets.SF_ToolbarButton
#   Aggregate
SERVICE = Union[ARRAY, BASIC, DICTIONARY, EXCEPTION, FILESYSTEM, L10N, PLATFORM, REGION, SESSION, STRING,
                TEXTSTREAM, TIMER, UI,
                DATABASE, DATASET, DATASHEET,
                DIALOG, DIALOGCONTROL,
                DOCUMENT, BASE, CALC, CALCREFERENCE, CHART, FORM, FORMCONTROL, FORMDOCUMENT, WRITER,
                MENU, POPUPMENU, TOOLBAR, TOOLBARBUTTON]
#   UNO
UNO = TypeVar('UNO')
#   Other
FILE = TypeVar('FILE', str, str)
""" File or folder name expressed in accordance with the ``FileSystem.FileNaming`` notation. """

SHEETNAME = TypeVar('SHEETNAME', str, str)
""" A sheet in a Calc document, as a string. It must not contain next characters: "[]*?:\\\/". The apostrophe (')
is forbidden in the first and last positions. The "~" (tilde) character designates the active sheet. """
RANGE = TypeVar('RANGE', str, str)
""" A set of contiguous cells located in a sheet.

The sheet name is optional. If no sheet name is provided, then the active sheet is used.
The use of single quotes to enclose the sheet name is required if the name contains blank spaces " " or periods ".".
Otherwise surrounding single quotes and $ signs are allowed but ignored.

The shortcut "~" (tilde) represents the current selection or the first selected range if multiple ranges are selected.
The shortcut "*" represents all used cells.

Examples
    - ``$'SheetX'.D2``, ``$D$2`` - Single cells
    - ``$'SheetX'.D2:F6``, ``D2:D10`` - Single ranges with multiple cells
    - ``$'SheetX'.*`` - All used cells in the given sheet
    - ``$'SheetX'.A:A``, ``3:5`` - All cells in contiguous columns or rows up to the last used cell
    - ``myRange`` - A range named "myRange" at spreadsheet level
    - ``~.someRange``, ``SheetX.someRange`` - Range names at sheet level
    - ``~.~`` or ``~`` - The current selection in the active sheet
"""
SCRIPT_URI = TypeVar('SCRIPT_URI', str, str)
""" String reference to a Basic or Python macro as described in
``https://wiki.documentfoundation.org/Documentation/DevGuide/Scripting_Framework#Scripting_Framework_URI_Specification``
"""
SQL_SELECT = TypeVar('SQL_SELECT', str, str)
""" A SQL command containing a SELECT statement, a tablename or a queryname.
In a SELECT statement, table-, qyery- and field names may be surrounded by square brackets. """
SQL_ACTION = TypeVar('SQL_ACTION', str, str)
""" A SQL command containing an action statement (CREATE TABLE, INSERT, DELETE, ...).
Table- and field names may be surrounded by square brackets. """
SCALAR = Union[int, float, str, datetime.datetime]
VECTOR = Sequence[SCALAR]
MATRIX = Sequence[VECTOR]

#   Define the used UNO types
try:
    # Next code can succeed only when types-unopy is installed
    from com.sun.star.awt import XControl
    from com.sun.star.awt import XControlModel
    from com.sun.star.awt import XTabControllerModel
    from com.sun.star.awt import XWindow
    from com.sun.star.awt.tree import XMutableTreeDataModel
    from com.sun.star.awt.tree import XMutableTreeNode
    from com.sun.star.beans import PropertyValue
    from com.sun.star.chart import XDiagram
    from com.sun.star.document import XEmbeddedScripts
    from com.sun.star.drawing import XShape
    from com.sun.star.form import ListSourceType
    from com.sun.star.form import XForm
    from com.sun.star.frame import XDesktop
    from com.sun.star.lang import XComponent
    from com.sun.star.script import XLibraryContainer
    from com.sun.star.script.provider import XScriptProvider
    from com.sun.star.sdb import XOfficeDatabaseDocument
    from com.sun.star.sdbc import XConnection as UNOXConnection
    from com.sun.star.sdbc import XDatabaseMetaData
    from com.sun.star.sheet import XSheetCellCursor
    from com.sun.star.sheet import XSpreadsheet
    from com.sun.star.table import XCellRange
    from com.sun.star.table import XTableChart
    from com.sun.star.uno import XComponentContext
    from com.sun.star.uno import XInterface
    from com.sun.star.util import Date as UNODate
    from com.sun.star.util import DateTime as UNODateTime
    from com.sun.star.util import Time as UNOTime
except ImportError:
    # types-unopy is not installed
    XControl = NewType('XControl', UNO)
    XControlModel = NewType('XControlModel', UNO)
    XTabControllerModel = NewType('XTabControllerModel', UNO)
    XWindow = NewType('XWindow', UNO)
    XMutableTreeDataModel = NewType('XMutableTreeDataModel', UNO)
    XMutableTreeNode = NewType('XMutableTreeNode', UNO)
    PropertyValue = NewType('PropertyValue', UNO)
    XDiagram = NewType('XDiagram', UNO)
    XEmbeddedScripts = NewType('XEmbeddedScripts', UNO)
    XShape = NewType('XShape', UNO)
    ListSourceType = NewType('ListSourceType', UNO)
    XForm = NewType('XForm', UNO)
    XDesktop = NewType('XDesktop', UNO)
    XComponent = NewType('XComponent', UNO)
    XLibraryContainer = NewType('XLibraryContainer', UNO)
    XScriptProvider = NewType('XScriptProvider', UNO)
    XOfficeDatabaseDocument = NewType('XOfficeDatabaseDocument', UNO)
    UNOXConnection = NewType('UNOXConnection', UNO)
    XDatabaseMetaData = NewType('XDatabaseMetaData', UNO)
    XSheetCellCursor = NewType('XSheetCellCursor', UNO)
    XSpreadsheet = NewType('XSpreadsheet', UNO)
    XCellRange = NewType('XCellRange', UNO)
    XTableChart = NewType('XTableChart', UNO)
    XComponentContext = NewType('XComponentContext', UNO)
    XInterface = NewType('XInterface', UNO)
    UNODate = NewType('UNODate', UNO)
    UNODateTime = NewType('UNODateTime', UNO)
    UNOTime = NewType('UNOTime', UNO)

# Docstring rules to display readable text both in PyCharm and VS Code
# Indentation and punctuation are required like in the example below.
#   def ImportFromCSVFile(self, filename: FILE, delimiter: str = ',',
#                       dateformat: Literal['YYYY-MM-DD', 'DD-MM-YYYY', 'MM-DD-YYYY'] = ...) -> Tuple[str, ...]:
#       """
#           Import the data contained in a comma-separated values (CSV) file. The comma may be replaced
#           by any character.
#           Difference with the Basic version: dates are returned in their iso format,
#           not as any of the datetime objects.
#               Args            <<<<< No colon (:)
#                   ``filename``: The name of the text file containing the data.
#                   The name must be expressed according to the current FileNaming property of the FileSystem
#                   service.
#                               <<<<< When multiple arguments, a linefeed is required here
#                   ``dateformat``: A special mechanism handles dates when ``dateformat`` is not the empty string.
#                   The dash (-) may be replaced by a dot (.), a slash (/) or a space. Other date formats
#                   will be ignored.
#               Returns         <<<<< No colon (:)
#                   A ``list`` of ``lists``.
#           """
#       ...


# #####################################################################################################################
#                           ScriptForge CLASS                                                                       ###
# #####################################################################################################################

class ScriptForge(object, metaclass = ...):
    """
        The ScriptForge class encapsulates the core of the ScriptForge run-time
            - Bridge with the LibreOffice process
            - Implementation of the interlanguage protocol with the Basic libraries
            - Identification of the available services interfaces
            - Dispatching of services
            - Coexistence with UNO

        The class may be instantiated several times. Only the first instance will be retained ("Singleton").
        """

    def __init__(self, hostname: str = ..., port: int = ...):
        """
            The ScriptForge class encapsulates the core of the ScriptForge run-time
                - Bridge with the LibreOffice process
                - Implementation of the interlanguage protocol with the Basic libraries
                - Identification of the available services interfaces
                - Dispatching of services
                - Coexistence with UNO

            The class may be instantiated several times. Only the first instance will be retained ("Singleton").

            Both arguments are mandatory when Python and LibreOffice run in separate processes.
            Otherwise, do not call this routine or leave both arguments out.
            To execute at most once by LibreOffice session.
                Args
                    ``hostname``: probably 'localhost'

                    ``port``: port number
            """
        ...


# #####################################################################################################################
#                           SFServices CLASS    (ScriptForge services superclass)                                   ###
# #####################################################################################################################

class SFServices(object):
    """
        Generic implementation of a parent Service class.

        Every service must subclass this class to be recognized as a valid service.
        A service instance is created by the ``CreateScriptService`` method.

        It can have a mirror in the ``Basic`` world or be totally defined in ``Python``.
        """

    def Dispose(self) -> None:
        """ Free up the object's resources after usage.    """
        ...

    def GetProperty(self, propertyname: str, arg: Any = ...) -> Any:
        """ Get the given property from the Basic world """
        ...

    def Properties(self) -> Tuple[str, ...]:
        """ Properties list.    """
        ...


# #####################################################################################################################
#                       SFScriptForge CLASS    (alias of ScriptForge Basic library)                                 ###
# #####################################################################################################################
class SFScriptForge:
    """ SF_ScriptForge all-purpose services. """

    # #########################################################################
    # SF_Array CLASS
    # #########################################################################
    class SF_Array(SFServices, metaclass = ...):

        def ImportFromCSVFile(self,
                              filename: FILE,
                              delimiter: str = ',',
                              dateformat: Literal['YYYY-MM-DD', 'DD-MM-YYYY', 'MM-DD-YYYY',
                                                    'YYYY/MM/DD', 'DD/MM/YYYY', 'MM/DD/YYYY',
                                                    'YYYY.MM.DD', 'DD.MM.YYYY', 'MM.DD.YYYY',
                                                    'YYYY MM DD', 'DD MM YYYY', 'MM DD YYYY'] = ...
                              ) -> MATRIX:
            """
                Import the data contained in a comma-separated values (CSV) file. The comma may be replaced
                by any character.
                Difference with the Basic version: dates are returned in their iso format,
                not as any of the datetime objects.
                    Args
                        ``filename``: The name of the text file containing the data.
                        The name must be expressed according to the current FileNaming property of the FileSystem
                        service.

                        ``dateformat``: A special mechanism handles dates when ``dateformat`` is not the empty string.
                        The dash (-) may be replaced by a dot (.), a slash (/) or a space. Other date formats
                        will be ignored.
                Returns
                        A ``list`` of ``lists``.
                """
            ...

    # #########################################################################
    # SF_Basic CLASS
    # #########################################################################
    class SF_Basic(SFServices, metaclass = ...):
        """
            This service proposes a collection of Basic methods to be executed in a Python context
            simulating the exact syntax and behavior of the identical Basic builtin method.\n
            Typical example:
                ``basic = CreateScriptService('basic')``
                ``basic.MsgBox('This has to be displayed in a message box')``
            """

        MB_ABORTRETRYIGNORE: Literal[2]
        MB_DEFBUTTON1: Literal[128]
        MB_DEFBUTTON2: Literal[258]
        MB_DEFBUTTON3: Literal[215]
        MB_ICONEXCLAMATION: Literal[48]
        MB_ICONINFORMATION: Literal[64]
        MB_ICONQUESTION: Literal[32]
        MB_ICONSTOP: Literal[16]
        MB_OK: Literal[0]
        MB_OKCANCEL: Literal[1]
        MB_RETRYCANCEL: Literal[5]
        MB_YESNO: Literal[4]
        MB_YESNOCANCEL: Literal[3]
        IDABORT: Literal[3]
        IDCANCEL: Literal[2]
        IDIGNORE: Literal[5]
        IDNO: Literal[7]
        IDOK: Literal[1]
        IDRETRY: Literal[4]
        IDYES: Literal[6]

        @classmethod
        def CDate(cls, datevalue: Union[int, float, str]) -> Optional[datetime.datetime]:
            """
                Converts a numeric expression or a string to a ``datetime.datetime`` Python native object.
                    Args
                        ``datevalue``: A numeric expression or a ``string`` representing a date.
                    Returns
                        The equivalent ``datetime.datetime``.
                """
            ...

        @staticmethod
        def CDateFromUnoDateTime(unodate: Union[UNODateTime, UNODate, UNOTime]) -> datetime.datetime:
            """
                Converts a ``UNO date/time`` representation to a ``datetime.datetime`` Python native object.
                    Args
                        ``unodate``: A ``UNO date`` object, ``com.sun.star.util.DateTime``, ``.Date`` or ``.Time``.
                    Returns
                        The equivalent ``datetime.datetime``.
                """
            ...

        @staticmethod
        def CDateToUnoDateTime(date: Union[float, time.struct_time, datetime.datetime, datetime.date, datetime.time]
                               ) -> UNODateTime:
            """
                Converts a date representation into the ``com.sun.star.util.DateTime`` date format.
                    Args
                        ``date``: A ``datetime`` like object.
                        When ``date`` is a ``float`` it is considered a ``time.time`` value.
                    Returns
                        A ``com.sun.star.util.DateTime`` UNO object.
                """
            ...

        @classmethod
        def ConvertFromUrl(cls, url: str) -> str:
            """
                Convert from url.
                    Args
                        ``url``: A string representing a file in URL format.
                    Returns
                        The same file name in native operating system notation._
                """
            ...

        @classmethod
        def ConvertToUrl(cls, systempath: str) -> str:
            """
                Convert to url.
                    Args
                        ``systempath``: A string representing a file in native operating system notation.
                    Returns
                        The same file name in URL format.
                """
            ...

        @classmethod
        def CreateUnoService(cls, servicename: str) -> UNO:
            """
                Instantiates a UNO service
                    Args
                        ``servicename``:  A string representing the service to create.
                    Returns
                        A UNO object
                """
            ...

        @classmethod
        def CreateUnoStruct(cls, unostructure: str) -> UNO:
            """
                Returns an instance of a UNO structure of the specified type.
                    Args
                        ``unostructure``: A UNO Struct typename such as ``com.sun.star.awt.Size``.
                    Returns
                        A UNO Object
            """
            ...

        @classmethod
        def DateAdd(cls,
                    interval: Literal['yyyy', 'q', 'm','y', 'w', 'ww', 'd', 'h', 'n', 's'],
                    number: int,
                    date: Union[float, time.struct_time, datetime.datetime, datetime.date, datetime.time]
                    ) -> datetime.datetime:
            """
                Adds a date or time interval to a given date/time a number of times and returns the resulting date.
                    Args
                        ``interval``: A string expression specifying the date or time interval.

                        ``number``: A numerical expression specifying how often the interval value will be added when
                        positive or subtracted when negative.

                        ``date``: A given ``datetime.datetime`` value, the interval value will be added ``number``
                        times to this ``date`` value.
                    Returns
                        A ``datetime.datetime`` value.
                """
            ...

        @classmethod
        def DateDiff(cls,
                     interval: Literal['yyyy', 'q', 'm','y', 'w', 'ww', 'd', 'h', 'n', 's'],
                     date1: Union[float, time.struct_time, datetime.datetime, datetime.date, datetime.time],
                     date2: Union[float, time.struct_time, datetime.datetime, datetime.date, datetime.time],
                     firstdayofweek: Literal[0, 1, 2, 3, 4, 5, 6, 7] = ...,
                     firstweekofyear: Literal[0, 1, 2, 3] = ...,
                     ) -> int:
            """
                Gets the number of date or time intervals between two given date/time values.
                    Args
                        ``interval``: A string expression specifying the date interval.

                        ``date1``: The first ``datetime.datetime`` values to be compared.

                        ``firstdayofweek``: An optional parameter that specifies the starting day of a week.

                        ``firstweekofyear``: An optional parameter that specifies the starting week of a year.
                    Returns
                        A number.
                """
            ...

        @classmethod
        def DatePart(cls,
                     interval: Literal['yyyy', 'q', 'm','y', 'w', 'ww', 'd', 'h', 'n', 's'],
                     date: Union[float, time.struct_time, datetime.datetime, datetime.date, datetime.time],
                     firstdayofweek: Literal[0, 1, 2, 3, 4, 5, 6, 7] = ...,
                     firstweekofyear: Literal[0, 1, 2, 3] = ...,
                     ) -> int:
            """
                Gets a specified part of a date.
                    Args
                        ``interval``: A string expression specifying the date interval.

                        ``date``: The date from which to extract a part.

                        ``firstdayofweek``: the starting day of a week. Defaults to 1.

                        ``firstweekofyear``: the starting week of a year. Defaults to 1.
                    Returns
                        The specified part of the date.
                """
            ...

        @classmethod
        def DateValue(cls, string: str) -> datetime.datetime:
            """
                Computes a date value from a date string.
                    Args
                        ``string``: A string expression that contains the date that you want to calculate.
                        The string passed to ``DateValue`` must be expressed in one of the date formats defined
                        by your locale setting or using the ISO date format "yyyy-mm-dd" (year, month and day
                        separated by hyphens).
                    Returns
                        The converted date.
                """
            ...

        @classmethod
        def Format(cls, expression: Union[datetime.datetime, float, int], format: str = ...) -> str:
            """
                Converts a number to a string, and then formats it according to the format that you specify.
                    Args
                        ``expression``: Numeric expression that you want to convert to a formatted string.

                        ``format``: the format to apply. Defaults to "".
                    Returns
                        The formatted value.
                """
            ...

        @classmethod
        def GetDefaultContext(cls) -> XComponentContext:
            """
                Gets the default context of the process service factory, if existent, else returns a None reference.
                """
            ...

        @classmethod
        def GetGuiType(cls) -> int:
            """
                Gets a numerical value that specifies the graphical user interface.
                    Returns
                        The GetGuiType value, 1 for Windows, 4 for UNIX
                """
            ...

        @classmethod
        def GetPathSeparator(cls) -> str:
            """
                Gets the operating system-dependent directory separator used to specify file paths.
                    Returns
                        The os path separator
                """
            ...

        @classmethod
        def GetSystemTicks(cls) -> int:
            """
                Gets the number of system ticks provided by the operating system.
                You can use this function to optimize certain processes.
                Use this method to estimate time in milliseconds:
                    Returns
                        The actual number of system ticks.
                """
            ...

        class GlobalScope(metaclass = ...):
            """
                Use cases:
                    - ``GlobalScope.BasicLibraries``
                    - ``GlobalScope.DialogLibraries``
                """

            @classmethod
            def BasicLibraries(cls) -> XLibraryContainer:
                """
                    ``GlobalScope.BasicLibraries`` gets the UNO object containing all shared Basic libraries
                    and modules. This method is the Python equivalent to ``GlobalScope.BasicLibraries``
                    in Basic scripts.
                        Returns
                            A ``XLibraryContainer`` UNO object.
                    """
                ...

            @classmethod
            def DialogLibraries(cls) -> XLibraryContainer:
                """
                    ``GlobalScope.DialogLibraries`` gets the UNO object containing all shared dialog libraries.
                        Returns
                            A ``DialogLibraryContainer`` UNO object.
                    """
                ...

        @classmethod
        def InputBox(cls,
                     prompt: str,
                     title: str = ...,
                     default: str = ...,
                     xpostwips: int = ...,
                     ypostwips: int = ...,
                     ) -> str:
            """
                Displays an input box.
                    Args
                        ``prompt``: String expression displayed as the message in the dialog box.

                        ``title``: String expression displayed in the title bar of the dialog box.

                        ``default``: String expression displayed in the text box as default ifno other input is given.

                        ``xpostwips``: Integer expression that specifies the horizontal position of the dialog.
                        The position is an absolute coordinate and does not refer to the window of LibreOffice.

                        ``ypostwips``: Integer expression that specifies the vertical position of the dialog.
                        The position is an absolute coordinate and does not refer to the window of LibreOffice.

                        If ``xpostwips`` and ``ypostwips`` are omitted, the dialog is centered on the screen.
                    Returns
                        The input string.
                """
            ...

        @classmethod
        def MsgBox(cls, prompt: str, buttons: int = ..., title: str = ...) -> int:
            """
                Displays a dialogue box containing a message and returns an optional value.

                MB_xx constants help specify the dialog type, the number and type of buttons to display,
                plus the icon type. By adding their respective values they form bit patterns, that define the
                MsgBox dialog appearance.
                    Args
                        ``prompt``: String expression displayed as a message in the dialog box.

                        ``buttons``: Integer expression that specifies the dialog type, as well as the number
                        and type of buttons to display, and the icon type. ``buttons`` represents a combination of bit
                        patterns, that is, a combination of elements can be defined by adding their respective values.
                        Defaults to 0.

                        ``title``: String expression displayed in the title bar of the dialog. Defaults to "".
                    Returns
                        The pressed button.
                """
            ...

        @classmethod
        def Now(cls) -> datetime.datetime:
            """
                Gets the current system date and time as a ``datetime.datetime`` Python native object.
                """
            ...

        @classmethod
        def RGB(cls, red: int, green: int, blue: int) -> int:
            """
                Gets an integer color value consisting of red, green, and blue components.
                    Args
                        ``red``: An integer expression that represents the red component (0-255) of the composite color.

                        ``green``: An integer expression that represents the green component (0-255) of the
                        composite color.

                        ``blue``: An integer expression that represents the blue component (0-255) of the composite
                        color.
                    Returns
                        An integer color value consisting of red, green, and blue components.
                """
            ...


        @classmethod
        def Xray(cls, unoobject: UNO) -> None:
            """
                Inspect UNO objects or variables.
                    Args
                        ``unoobject``: A variable or UNO object.
                """
            ...

        StarDesktop: XDesktop
        """ Gets the desktop as a UNO object. """
        ThisComponent: Optional[XComponent]
        """
            If the current component refers to a LibreOffice document, this method
            returns the UNO object representing the document.
            When the current component is the Basic IDE, the ThisComponent object returns in Basic the
            component owning the currently run user script. This behaviour cannot be reproduced in Python.
                Returns
                    The current component or None when not a document.
            """
        ThisDatabaseDocument: Optional[XOfficeDatabaseDocument]
        """
            If the script is being executed from a Base document or any of its subcomponents
            this method returns the main component of the Base instance.
                Returns
                    The current Base (main) component or None when not a Base document or one of its subcomponents.
            """

    # #########################################################################
    # SF_Dictionary CLASS
    # #########################################################################
    class SF_Dictionary(SFServices, dict):
        """
        The service adds to a Python dict instance the interfaces for conversion to and from
        a list of UNO PropertyValues
            Usage
                ``dico = dict(A = 1, B = 2, C = 3)``

                ``myDict = CreateScriptService('Dictionary', dico)    # Initialize myDict with the content of dico``

                ``myDict['D'] = 4``

                ``print(myDict)   # {'A': 1, 'B': 2, 'C': 3, 'D': 4}``

                ``propval = myDict.ConvertToPropertyValues()``
            or
                ``dico = dict(A = 1, B = 2, C = 3)``

                ``myDict = CreateScriptService('Dictionary')          # Initialize myDict as an empty dict object``

                ``myDict.update(dico) # Load the values of dico into myDict``

                ``myDict['D'] = 4``

                ``print(myDict)   # {'A': 1, 'B': 2, 'C': 3, 'D': 4}``

                ``propval = myDict.ConvertToPropertyValues()``
            """

        def ConvertToPropertyValues(self) -> Tuple[PropertyValue]:
            """
                Store the content of the dictionary in an array of PropertyValues.
                Each entry in the list is a ``com.sun.star.beans.PropertyValue``.
                The key is stored in Name, the value is stored in ``Value``.

                If one of the items has a type ``datetime``, it is converted to a ``com.sun.star.util.DateTime``
                structure. If one of the items is an empty list, it is converted to ``None``.
                    Returns
                        A list of property values. The resulting list is empty when the dictionary is empty.
                """
            ...

        def ImportFromPropertyValues(self, propertyvalues: Sequence[PropertyValue], overwrite: bool = ...) -> bool:
            """
                Inserts the contents of an array of ``PropertyValue`` objects into the current dictionary.
                ``PropertyValue`` Names are used as keys in the dictionary, whereas Values contain
                the corresponding values. Date-type values are converted to ``datetime.datetime`` instances.
                    Args
                        ``propertyvalues``: tuple containing ``com.sun.star.beans.PropertyValue`` objects.

                        ``overwrite``: When True, entries with same name may exist in the dictionary
                        and their values are overwritten. When ``False`` (default), repeated keys are not overwritten.
                        Defaults to ``False``.
                    Returns
                        True when successful.
                """
            ...

    # #########################################################################
    # SF_Exception CLASS
    # #########################################################################
    class SF_Exception(SFServices, metaclass = ...):
        """
            The Exception service is a collection of methods for code debugging.

            The ``Exception`` service console stores events, variable values and information about errors.

            Use the console when the Python shell is not available, for example in ``Calc`` user defined functions (UDF)
            or during events processing.

            Use the ``DebugPrint()`` and ``DebuDisplay()`` methods to log any relevant information, events  and data
            of any type to the console.

            Console entries can be dumped to a text file or visualized in a dialogue.
            """

        def Console(self, modal: bool = ...):
            """
                Displays the console messages in a modal or non-modal dialog. In both modes, all the past messages
                issued by a ``DebugPrint()`` method or resulting from an exception are displayed. In non-modal mode,
                subsequent entries are added automatically.

                If the console is already open, when non-modal, it is brought to the front.

                A modal console can only be closed by the user. A non-modal console can either be closed by the user
                or upon script termination.
                    Args
                        ``modal``: determine if the console window is modal (``True``) or non-modal (``False``).
                        Default value is ``True``.
                """
            ...

        def ConsoleClear(self, keep: int = ...):
            """
                Clears the console keeping an optional number of recent messages. If the console is activated in non-modal mode,
                it is refreshed.
                    Args
                        ``keep``: the number of recent messages to be kept. Default value is 0.
            """
            ...

        def ConsoleToFile(self, filename: FILE) -> bool:
            """
                Exports the contents of the console to a text file. If the file already exists and the console
                is not empty, it will be overwritten without warning. Returns True if successful.
                    Args
                        ``filename``: the name of the text file the console should be dumped into.
                        The name is expressed according to the current ``FileNaming`` property of the ``SF_FileSystem``
                         service.
                    Returns
                        ``True`` when successful.
                """
            ...

        def DebugDisplay(self, *args)-> int:
            """
                Concatenates all the arguments into a single human-readable string and displays it in a MsgBox
                with an Information icon and an OK button. The displayed string is also added to the Console.
                    Args
                        ``*args``: any number of arguments of any type.
                    Returns
                        The result of the execution of the ``MsgBox()`` method.
                """
            ...

        def DebugPrint(self, *args):
            """
                Assembles all the given arguments into a single human-readable string and adds it as a new entry
                in the console.
                    Args
                        ``*args``: any number of arguments of any type.
                """
            ...

        @classmethod
        def PythonShell(cls, variables: dict = ...) -> None:
            """
                Opens an APSO Python shell as a non-modal window.
                The Python script keeps running after the shell is opened.
                The output from ``print`` statements inside the script are shown in the shell.
                    Args
                        ``variables``: a Python dictionary with variable names and values that will be
                        passed on to the APSO Python shell. By default, all local variables are passed using
                        Python's builtin ``locals()`` function.

                        Typical use: ``{**globals(), **locals()}``
                """
            ...

    # #########################################################################
    # SF_FileSystem CLASS
    # #########################################################################
    class SF_FileSystem(SFServices, metaclass = ...):
        """
            The "FileSystem" service includes common file and folder handling routines.
            """

        FileNaming: str
        """
            Sets or returns the current files and folders notation, either ``'ANY', 'URL'`` or ``'SYS'``:
            
                ``ANY``: (default) the methods of the ``FileSystem`` service accept both URL and current operating 
                system's notation for input arguments but always return URL strings.
                
                ``URL``: the methods of the ``FileSystem`` service expect URL notation for input arguments and 
                return URL strings.
                
                ``SYS``: the methods of the ``FileSystem`` service expect current operating system's notation 
                for both input arguments and return strings.
            
            Once set, the ``FileNaming`` property remains unchanged either until the end of the ``LibreOffice``
            session or until it is set again.
            """
        ConfigFolder: FILE
        """ Returns the configuration folder of ``LibreOffice``. """
        ExtensionsFolder: FILE
        """ Returns the folder where extensions are installed. """
        HomeFolder: FILE
        """ Returns the user home folder. """
        InstallFolder: FILE
        """ Returns the installation folder of ``LibreOffice``. """
        TemplatesFolder: FILE
        """ Returns the folder containing the system templates files. """
        TemporaryFolder: FILE
        """ Returns the temporary files folder defined in the ``LibreOffice`` path settings. """
        UserTemplatesFolder: FILE
        """ Returns the folder containing the user-defined template files. """

        ForReading: Literal[1]
        ForWriting: Literal[2]
        ForAppending: Literal[8]
        
        def BuildPath(self, foldername: FILE,  name: str) -> str:
            """
                Joins a folder path and the name of a file and returns the full file name with a
                valid path separator. The path separator is added only if necessary.

                Args
                    ``foldername``: the path with which name will be combined.
                    The specified path does not need to be an existing folder.

                    ``name``: the name of the file to be appended to foldername. This parameter uses
                    the notation of the current operating system.

                Returns
                    The path concatenated with the file name after insertion of a path separator, if necessary.
                """
            ...

        def CompareFiles(self, filename1: FILE, filename2: FILE, comparecontents: bool = ...) -> bool:
            """
                Compare 2 files and return ``True`` if they seem identical.
                Depending on the value of the ``comparecontents`` argument,
                the comparison between both files can be either based only on
                file attributes (such as the last modified date), or based on the file contents.
                    Args
                        ``filename1``: the 1st file to compare.

                        ``filename2``: the 2nd file to compare.

                        ``comparecontents``: when ``True``, the contents of the files are compared.
                        Defaults to ``False``.
                    Returns
                        ``True`` when the files seem identical
                """
            ...

        def CopyFile(self, source: FILE, destination: FILE, overwrite: bool = ...) -> bool:
            """
                Copies one or more files from one location to another.
                    Args
                        ``source``: ``FileName`` or ``NamePattern`` which can include wildcard characters,
                        for one or more files to be copied.

                        ``destination``: ``FileName`` where the single ``source`` file is to be copied
                        or ``FolderName`` where the multiple files from ``source`` are to be copied.

                        ``overwrite``: if ``True`` (default), files may be overwritten.
                        ``CopyFile`` will fail if Destination has the read-only attribute set,
                        regardless of the value of Overwrite.
                    Returns
                        ``True`` if at least one file has been copied. ``False`` if an error occurred.
                        An error also occurs if a source using wildcard characters doesn't match any files.
                        The method stops on the first error it encounters.
                        No attempt is made to roll back or undo any changes made before an error occurs.
                    Notes
                        - If ``destination`` does not exist, it is created.
                        - Wildcard characters are not allowed in ``destination``.
                """
            ...

        def CopyFolder(self, source: FILE, destination: FILE, overwrite: bool = ...) -> bool:
            """
                Copies one or more folders from one location to another.
                    Args
                        ``source``: ``FolderName`` or ``NamePattern`` which can include wildcard characters,
                        for one or more folders to be copied.

                        ``destination``: ``FolderName`` where the single ``source`` folder is to be copied
                        or ``FolderName`` where the multiple folders from ``source`` are to be copied.
                        If ``FolderName`` does not exist, it is created.

                        ``overwrite``: if ``True`` (default), folders and their content may be overwritten.
                        Defaults to ``True``. ``CopyFile`` will fail if ``destination`` has the read-only
                        attribute set, regardless of the value of ``overwrite``.
                    Returns
                        ``True`` if at least one folder has been copied. ``False`` if an error occurred.
                        An error also occurs if a ``source`` using wildcard characters doesn't match any folders.
                        The method stops on the first error it encounters.
                        No attempt is made to roll back or undo any changes made before an error occurs.
                    Notes
                        - If ``destination`` does not exist, it is created.
                        - Wildcard characters are not allowed in ``destination``.
                """
            ...

        def CreateFolder(self, foldername: FILE) -> bool:
            """
                Creates the specified ``olderame``.
                If the specified folder has a parent folder that does not exist, it is created.
                    Args
                        foldername: a string representing the folder to create. It must not exist.
                    Returns
                        ``True`` if ``foldername`` is a valid folder name, does not exist and creation was successful.
                        ``False`` otherwise, including when ``foldername`` is a file.
                """
            ...

        def CreateTextFile(
            self, filename: FILE, overwrite: bool = ..., encoding: str = ...
        ) -> TEXTSTREAM:
            """
                Creates a specified file and returns a ``TextStream`` service instance that can be used
                to write to the file.
                The method returns ``None`` if an error occurred.
                    Args
                        ``filename``: the name of the file to be created.

                        ``overwrite``: determines if ``filename`` can be overwritten (default = ``True``).

                        ``encoding``: the character set to be used. The default encoding is "UTF-8".
                """
            ...

        def DeleteFile(self, filename: FILE) -> bool:
            """
                Deletes one or more files.
                The files to be deleted must not be readonly.
                    Args
                        ``filename``: ``FileName`` or ``NamePattern`` which can include wildcard characters,
                        for one or more files to be deleted.
                    Returns
                        ``True`` if at least one file has been deleted. ``False`` if an error occurred.
                        An error also occurs if ``fileName`` using wildcard characters doesn't match any files.
                        The method stops on the first error it encounters.
                        No attempt is made to roll back or undo any changes made before an error occurs.
                    """
            ...

        def DeleteFolder(self, foldername: FILE) -> bool:
            """
                Deletes one or more folders.
                The folders to be deleted must not be readonly.
                    Args
                        ``foldername``: ``FolderName`` or ``NamePattern`` which can include wildcard characters,
                        for one or more Folders to be deleted.
                    Returns
                        ``True`` if at least one folder has been deleted. ``False`` if an error occurred.
                        An error will also occur if the ``foldername`` parameter uses wildcard characters and
                        does not match any folders.
                        The method stops immediately after it encounters an error.
                        The method does not roll back nor does it undo changes made before the error occurred.
                """
            ...

        def ExtensionFolder(self, extension: str) -> str:
            """
                Returns a string containing the folder where the specified extension package is installed.
                    Args
                        ``extension``: a string value with the ID of the extension.
                        If the extension is not installed, an exception is raised.
                """
            ...

        def FileExists(self, filename: FILE) -> bool:
            """
                Return ``True`` if the given file exists.
                    Args
                        ``filename``: a string representing the file to be tested.
                    Returns
                        ``True`` if ``filename`` is a valid File name and it exists.
                        ``False`` otherwise, including when ``filename`` is a folder.
                """
            ...

        def Files(self, foldername: FILE, filter: str = ..., includesubfolders: bool = ...) -> Tuple[str, ...]:
            """
                Gets a tuple of the ``FileNames`` stored in the given folder.
                If the argument ``foldername`` specifies a folder that does not exist, an exception is raised.
                The resulting tuple may be filtered with wildcards.
                    Args
                        ``foldername``: the folder to explore.

                        ``filter``: contains wildcards ("?" and "*") to limit the list to the relevant files
                        (default = "").

                        ``includesubfolders``: set this argument to ``True`` to include the contents of
                        subfolders (Default = ``False``).
                    Returns
                        A tuple of strings, each entry is the ``FileName`` of an existing file.
                """
            ...

        def FolderExists(self, foldername: FILE) -> bool:
            """
                Returns ``True`` if the given folder name exists.
                    Args
                        ``foldername``: a string representing a folder.
                    Returns
                        ``True`` if ``folderName`` is a valid folder name and it exists.
                        ``False`` otherwise, including when ``foldername`` is a file.
                """
            ...

        def GetBaseName(self, filename: FILE) -> str:
            """
                Returns the ``BaseName`` (equal to the last component) of a file or folder name, without its extension.
                The method does not check for the existence of the specified file or folder.
                    Args
                        ``filename``: path and file name
                    Returns
                        The ``BaseName`` of the given file name. It may be the empty string.
                """
            ...

        def GetExtension(self, filename: FILE) -> str:
            """
                Returns the extension part of a ``File-`` or ``FolderName``, without the dot (.).
                The method does not check for the existence of the specified file or folder.
                    Args
                        ``filename``: path and file name
                    Returns
                        The extension without a leading dot. May be empty.
                """
            ...

        def GetFileLen(self, filename: FILE) -> int:
            """
                Returns the length of a file in bytes.
                    Args
                        ``filename``: a string representing an existing file.
                    Returns
                        File size if ``filename`` exists.
                """
            ...

        def GetFileModified(self, filename: FILE) -> datetime.datetime:
            """
                Returns the last modified date for the given file.
                    Args
                        ``filename``: a string representing an existing file.
                    Returns
                        The modification date and time.
                """
            ...

        def GetName(self, filename: FILE) -> str:
            """
                Returns the last component of a ``File-`` or ``FolderName``.
                The method does not check for the existence of the specified file or folder.
                    Args
                        ``filename``: path and file name
                    Returns
                        The last component of the full file name in native operating system format.
                """
            ...

        def GetParentFolderName(self, filename: FILE) -> FILE:
            """
            Returns a string containing the name of the parent folder of the last component in a specified
            ``File-`` or ``FolderName``.
            The method does not check for the existence of the specified file or folder.
                Args
                    ``filename``: path and file name.
                Returns
                    A folder name including its final path separator.
            """
            ...

        def GetTempName(self, extension: str = ...) -> FILE:
            """
                Returns a randomly generated temporary file name that is useful for performing
                operations that require a temporary file : the method does not create any file.
                    Args
                        ``extension``: the extension of the temporary file name (Default = "").
                    Returns
                        A ``FileName`` as a string.
                        By default, the returned file name does not have an extension (suffix).
                        Use the extension parameter to specify the extension of the file name to be generated.
                        The folder part of the returned string is the system's temporary folder.
                """
            ...

        def HashFile(self,
                     filename: FILE,
                     algorithm: Literal['MD5', 'SHA1', 'SHA224', 'SHA256', 'SHA384', 'SHA512'],
                     ) -> str:
            """
                Gets a hexadecimal string representing a checksum of the given file.
                    Args
                        ``filename``: a string representing a file.

                        ``algorithm``: the hashing algorithm to use.
                    Returns
                        The requested checksum as a string. Hexadecimal digits are lower-cased.
                        A zero-length string when an error occurred.
                    Note
                        The supported hashing algorithms are:
                            - MD5
                            - SHA1
                            - SHA224
                            - SHA256
                            - SHA384
                            - SHA512
                """
            ...

        def MoveFile(self, source: FILE, destination: FILE) -> bool:
            """
                Moves one or more files from one location to another.
                Returns ``True`` if at least one file has been moved or ``False`` if an error occurred.
                An error will also occur if the source parameter uses wildcard characters and does not match any files.

                The method stops immediately after it encounters an error. The method does not
                roll back nor does it undo changes made before the error occurred.
                    Args
                        ``source``: ``FileName`` or ``NamePattern`` which can include wildcard characters,
                        for one or more files to be moved.

                        ``destination``: if ``source`` is an existing file name then ``destination`` indicates
                        the new path and file name of the moved file. If the move operation involves multiple files,
                        then ``destination`` must be a folder name. If it does not exist, it is created.
                        If ``source`` and ``destination`` have the same parent folder, the method will rename
                        the ``source``.
                        Wildcard characters are not allowed in ``destination``.
                    Returns
                        ``True`` if at least one file has been moved. ``False`` if an error occurred.
                """
            ...

        def MoveFolder(self, source: FILE, destination: FILE) -> bool:
            """
                Moves one or more folders from one location to another.
                Returns ``True`` if at least one folder has been moved or ``False`` if an error occurred.
                An error will also occur if the source parameter uses wildcard characters and does
                not match any folders.

                The method stops immediately after it encounters an error. The method does not roll
                back nor does it undo changes made before the error occurred.
                    Args
                        ``source``: ``FolderName`` or ``NamePattern`` which can include wildcard characters,
                        for one or more folders to be moved.

                        ``destination``: if the move operation involves a single folder, then ``destination``
                        is the name and path of the moved folder. It must not exist.
                        If multiple folders are being moved, then ``destination`` designates where the folders
                        in ``source`` will be moved into. If ``destination`` does not exist, it is created.
                        Wildcard characters are not allowed in ``destination``.
                    Returns
                        ``True`` if at least one folder has been moved. ``False`` if an error occurred.
                """
            ...

        def Normalize(self, filename: FILE) -> FILE:
            """
                Returns a string containing the normalized path name by collapsing redundant separators and up-level
                references. For instance, the path names ``A//B``, ``A/B/``, ``A/./B`` and ``A/foo/../B`` are all
                normalized to ``A/B``.

                On Windows, forward slashes ``/`` are converted to backward slashes \\\.
                    Args
                        ``filename``: a string representing a valid path name.
                        The file or directory represented by this argument may not exist.
                    Returns
                        The normalized file name.
                """
            ...

        def OpenTextFile(self,
                         filename: FILE,
                         iomode: Literal[1, 2, 8] = ...,
                         create: bool = ...,
                         encoding: str = ...,
                         ) -> TEXTSTREAM:
            """
                Opens a specified file and returns a ``TextStream`` object that can be used to read from,
                write to, or append to the file.
                    Args
                        ``filename``: identifies the file to open.

                        ``iomode``: indicates input/output mode. It can be one of three constants:
                        ``filesystem.ForReading`` (default), ``filesystem.ForWriting``, or ``filesystem.ForAppending``.

                        ``create``: boolean value that indicates whether a new file can be created if the specified
                        filename doesn't exist. The value is ``True`` if a new file and its parent folders
                        may be created. ``False`` if they aren't created. Defaults to ``False``.

                        ``encoding``: the character set that should be used. Defaults to "UTF-8".
                    Returns
                        A ``TEXTSTREAM`` instance representing the opened file or ``None`` if an error occurred.
                        The method does not check if the file is really a text file. It doesn't check either
                        if the given encoding is implemented in LibreOffice nor if it is the right one.
                """
            ...

        def PickFile(self,
                     defaultfile: FILE = ...,
                     mode: Literal['OPEN', 'SAVE'] = ...,
                     filter: str = ...,
                     ) -> FILE:
            """
                Opens a dialog box to open or save a file.

                If the ``SAVE`` mode is set and the picked file exists, a warning message will be displayed.
                    Args
                        ``defaultfile``: this argument is a string composed of a folder and file name.
                        The folder part indicates the folder that will be shown when the dialog opens
                        (default = the last selected folder). The file part designates the default file to open or save.

                        ``mode``: a string value that can be either "OPEN" (for input files) or
                        "SAVE" (for output files). The default value is "OPEN".

                        ``filter``: the extension of the files displayed when the dialog is opened
                        (default = no filter).
                    Returns
                        The selected FileName in ``filesystem.FileNaming`` format or "" if the dialog was cancelled.
                """
            ...

        def PickFolder(self, defaultfolder: FILE = ..., freetext: str = ...) -> FILE:
            """
                Display a dialog box to select a folder.
                    Args
                        ``defaultfolder``: the folder from which to start. Default = the last selected folder.

                        ``freetext``: text to display in the dialog. Defaults to "".
                    Returns
                        The selected folder in ``filesystem.FileNaming`` notation.
                        A zero-length string if the dialog was cancelled.
                """
            ...
        def SubFolders(self,
                       foldername: FILE,
                       filter: str = ...,
                       includesubfolders: bool = ...
                       ) -> Tuple[str, ...]:
            """
                Returns a tuple of strings corresponding to the folders stored
                in the given ``foldername``.
                The list may be filtered with wildcards.
                    Args
                        ``foldername``: a string representing a folder. The folder must exist. ``foldername`` must not
                        designate a file.

                        ``filter``: a string containing wildcards ("?" and "*") that will be applied to the resulting
                        list of folders (default = "").

                        ``includesubfolders``: set this argument to ``True`` to include the contents of
                        subfolders (Default = ``False``).
                """
            ...

    # #########################################################################
    # SF_L10N CLASS
    # #########################################################################
    class SF_L10N(SFServices):
        """
            This service provides a number of methods related to the translation of strings
            with minimal impact on the program's source code.

            The methods provided by the L10N service can be used mainly to:
                - Create POT files that can be used as templates for translation of all strings in the program.
                - Get translated strings at runtime for the language defined in the Locale property.
            """

        Folder: FILE
        """ The folder containing the ``PO`` files  """
        Languages: Tuple[str, ...]
        """ A list of all the base names (without the ``.po`` extensions found in the specified ``folder``. """
        Locale: str
        """ The currently active ``language-COUNTRY`` combination.  """

        def AddText(self, context: str = ..., msgid: str = ..., comment: str = ...) -> bool:
            """
                Adds a new entry in the list of localizable strings. It must not exist yet.
                    Args
                        ``context``: the key to retrieve the translated string with the GetText method.
                        This parameter has a default value of "".

                        ``msgid``: the untranslated string, which is the text appearing in the program code. ``msgid``
                        must not be empty.

                        The ``msgid`` becomes the key to retrieve the translated string via
                        the ``GetText()`` method when ``context`` is empty.
                        The ``msgid`` string may contain any number of placeholders (%1 %2 %3 ...) for dynamically
                        modifying the string at runtime.

                        ``comment``: optional comment to be added alongside the string to help translators.
                    Returns
                        ``True`` if successful.
                """
            ...

        def AddTextsFromDialog(self, dialog: DIALOG) -> bool:
            """
                Add all fixed text strings of a dialog to the list of localizable text strings.

                Added texts are:
                    - The title of the dialog.
                    - The caption associated with next control types: Button, CheckBox, FixedLine, FixedText, GroupBox and RadioButton.
                    - The content of list- and combo boxes.
                    - The tip- or help text displayed when the mouse is hovering the control.

                The current method has method ``SFDialogs.SF_Dialog.GetTextsFromL10N()`` as counterpart.
                The targeted dialog must not be open when the current method is run.
                    Args
                        ``dialog``: a SFDialogs.Dialog service instance
                    Returns
                        ``True`` when successful.
                """
            ...

        def ExportToPOTFile(self, filename: FILE, header: str = ..., encoding: str = ...) -> bool:
            """
                Export a set of untranslated strings as a POT file.
                The set of strings has been built either by a succession of AddText() methods
                or by a successful invocation of the L10N service with the ``FolderName`` argument.
                The generated file should pass successfully the ``"msgfmt --check 'the pofile'"`` GNU command.
                    Args
                        ``filename``: the complete file name (in ``filesystem.FileNaming`` notation)
                        to export to. If it exists, it is overwritten without warning.

                        ``header``: comments that will appear on top of the generated file. Do not include
                        any leading "#". If the string spans multiple lines, insert escape sequences (``\\n``) where
                        relevant. A standard header will be added anyway. Defaults to "".

                        ``encoding``: the character set that should be used. Defaults to "UTF-8".
                    Returns
                        ``True`` if successful.
                """
            ...

        def GetText(self, msgid: str, *args: Any) -> str:
            """
                Gets the translated string corresponding to the given ``msgid`` argument.
                A list of arguments may be specified to replace the placeholders (%1, %2, ...) in the string.
                    Args
                        ``msgid``: the untranslated string, which is the text appearing in the program code.
                        It must not be empty. It may contain any number of placeholders (%1 %2 %3 ...) that can be used
                        to dynamically insert text at runtime.

                        Besides using a single ``msgid`` string, this method also accepts the following formats:
                            - the untranslated text (``msgid``)
                            - the reference to the untranslated text (``context``)
                            - both (``context|msgId``), use the pipe character as delimiter.

                        ``args``: values to be inserted into the placeholders.
                        Any variable type is allowed, however only strings, numbers and dates will be considered.
                    Returns
                        The translated string. If no translated string is found, the method returns the untranslated
                        string after replacing the  placeholders with the specified arguments.
                """
            ...

    # #########################################################################
    # SF_Platform CLASS
    # #########################################################################
    class SF_Platform(SFServices, metaclass = ...):
        """
            The ``Platform`` service implements a collection of properties about the actual execution environment
            and context, such as :
                - the hardware platform
                - the operating system
                - the LibreOffice version
                - the current user
            All its properties are read-only.
            """

        Architecture: str
        """ The hardware bit architecture. Example: '32bit' or '64bit'. """
        ComputerName: str
        """ The computer's network name.    """
        CPUCount: int
        """ The number of central processing units. """
        CurrentUser: str
        """ The name of the currently logged user.  """
        Extensions: Tuple[str, ...]
        """ An array of strings containing the internal IDs of all installed extensions.    """
        FilterNames: Tuple[str, ...]
        """ An array of strings containing the available document import and export filter names.   """
        Fonts: Tuple[str, ...]
        """ An array of strings containing the names of all available fonts.    """
        FormatLocale: str
        """ The locale used for numbers and dates as a string in the format "la-CO" (language-COUNTRY). """
        Locale: str
        """ The locale of the operating system as a string in the format "la-CO" (language-COUNTRY).
        This is equivalent to the ``SystemLocale`` property.    """
        Machine: str
        """ The machine type. Examples are: 'i386' or 'x86_64'. """
        OfficeLocale: str
        """ The locale of the user interface as a string in the format "la-CO" (language-COUNTRY).  """
        OfficeVersion: str
        """ The actual LibreOffice version expressed as ``LibreOffice w.x.y.z (The Document Foundation)``.    """
        OSName: str
        """ The operating system type. Example: 'Darwin, Linux' or 'Windows'.   """
        OSPlatform: str
        """ A single string identifying the underlying platform with as much useful and human-readable information
        as possible. Example: ``Linux-5.8.0-44-generic-x86_64-with-glibc2.32``. """
        OSRelease: str
        """ The operating system's release. Example: ``5.8.0-44-generic``.    """
        OSVersion: str
        """ The operating system's build or version. Example:  ``#50-Ubuntu SMP Tue Feb 9 06:29:41 UTC 2021 ``  """
        Printers: Tuple[str, ...]
        """ The list of available printers. The default printer is put in the first position of the list
        (index = [0]).  """
        Processor: str
        """ The real processor name. Example: ``amdk6``. This property may return the same value as the ``Machine``
        property.   """
        PythonVersion: str
        """ The version of the Python interpreter being used as a string in the format ``Python major.minor.patchlevel``
        (ex: ``Python 3.9.7``).   """
        SystemLocale: str
        """ The locale of the operating system as a string in the format "la-CO" (language-COUNTRY).
        This is equivalent to the ``Locale`` property.  """
        UserData: DICTIONARY
        """ Returns a ``Dictionary`` instance containing key-value pairs in relation with the
        ``Tools - Options - User Data`` dialog. """

    # #########################################################################
    # SF_Region CLASS
    # #########################################################################
    class SF_Region(SFServices, metaclass = ...):
        """
            The "Region" service gathers a collection of functions about languages, countries and timezones
                - Locales
                - Currencies
                - Numbers and dates formatting
                - Calendars
                - Timezones conversions
                - Numbers transformed to text in any supported language
            """

        def Country(self, region: str = ...) -> str:
            """
                Gets the country name in English corresponding to a given region.
                    Args
                        ``region``: formatted as "la-CO" or "CO".
                """
            ...

        def Currency(self, region: str = ...) -> str:
            """
                Gets the ISO 4217 currency code of the specified region.
                    Args:
                        ``region``: formatted as "la-CO" or "CO".
                """
            ...

        def DatePatterns(self, region=...) -> Tuple[str, ...]:
            """
            Gets an array of strings containing the date acceptance patterns for the specified region.
                Args
                    ``region``: formatted as "la-CO".
                """
            ...

        def DateSeparator(self, region: str = ...) -> str:
            """
                Returns the date separator used in the given region.
                    Args
                        ``region``: formatted as "la-CO".
                """
            ...

        def DayAbbrevNames(self, region: str = ...) -> Tuple[str, ...]:
            """
                Gets an array of strings containing the list of abbreviated weekday names in the specified language.
                    Args
                        ``region``: formatted as "la-CO" or "la".
                """
            ...

        def DayNames(self, region: str = ...) -> Tuple[str, ...]:
            """
                Gets an array of strings containing the list of weekday names in the specified language.
                    Args
                        ``region``: formatted as "la-CO" or "la".
                """
            ...

        def DayNarrowNames(self, region: str = ...) -> Tuple[str, ...]:
            """
                Gets a zero-based array of strings containing the list of the initials of weekday names
                in the specified language.
                    Args
                        ``region``: formatted as "la-CO" or "la".
                """
            ...

        def DecimalPoint(self, region: str = ...) -> str:
            """
                Gets the decimal separator used in numbers in the specified region.
                    Args
                        ``region``: formatted as "la-CO".
                """
            ...

        def Language(self, region: str = ...) -> str:
            """
                Gets tthe name of the language, in English, of the specified region.
                    Args
                        ``region``: formatted as "la-CO" or "la".
                """
            ...

        def ListSeparator(self, region: str = ...) -> str:
            """
                Gets the list separator used in the specified region.
                    Args
                        ``region``: formatted as "la-CO".
                """
            ...

        def MonthAbbrevNames(self, region: str = ...) -> Tuple[str, ...]:
            """
                Gets a zero-based array of strings containing the list of abbreviated month names
                in the specified language.
                    Args
                        ``region``: formatted as "la-CO".
                """
            ...

        def MonthNames(self, region: str = ...) -> Tuple[str, ...]:
            """
                Gets an array of strings containing the list of month names in the specified language.
                    Args
                        ``region``: formatted as "la-CO" or "la".
                """
            ...

        def MonthNarrowNames(self, region: str = ...) -> Tuple[str, ...]:
            """
                Gets an array of strings containing the list of the initials of month names in the specified language.
                    Args
                        ``region``: formatted as "la-CO" or "la".
                """
            ...

        def ThousandSeparator(self, region: str = ...) -> str:
            """
                Gets the thousands separator used in numbers in the specified region.
                    Args
                        ``region``: formatted as "la-CO".
                """
            ...

        def TimeSeparator(self, region: str = ...) -> str:
            """
                Gets the separator used to format times in the specified region.
                    Args
                        ``region``: formatted as "la-CO".
                """
            ...

        def DSTOffset(self, localdatetime: datetime.datetime, timezone: str, locale: str = ...) -> int:
            """
                Computes the additional Daylight Saving Time (DST) offset, in minutes, that is applicable
                to a given region and timezone.
                    Args
                        ``localdatetime``: the local date and time to consider.

                        ``timezone``: the timezone for which the offset will be calculated.

                        ``locale``: the locale specifying the country for which the offset will be calculated,
                        given either in "la-CO" or "CO" formats. The default value is the locale defined
                        in the ``OfficeLocale`` property of the ``Platform`` service.
                """
            ...

        def LocalDateTime(self, utcdatetime: datetime.datetime, timezone: str, locale: str = ...) -> datetime.datetime:
            """
                Computes the local date and time from a UTC date and time.
                    Args
                        ``utcdatetime``: the UTC date and time, expressed using a date object.

                        ``timezone``: the timezone for which the local time will be calculated.

                        ``locale``: the locale specifying the country for which the offset will be calculated,
                        given either in "la-CO" or "CO" formats. The default value is the locale defined
                        in the ``OfficeLocale`` property of the ``Platform`` service.
                """
            ...

        def Number2Text(self, number: Union[int, float, str], locale: str = ...) -> str:
            """
                Converts numbers and monetary values into written text for any of the currently supported languages.
                    Args
                        ``number``: the number to be converted into written text.
                        It can be provided either as a numeric type or as a string. When a string is provided,
                        it can be preceded by a prefix informing how the numbers should be written.
                        It is also possible to include ISO 4217 currency codes.

                        ``locale``: the locale defining the language into which the number will be converted to,
                        given either in "la-CO" or "la" formats.
                        The default value is the locale defined in the ``OfficeLocale`` property of the ``Platform``
                        service.
                    Returns
                        The converted number or monetary value in letters.
                """
            ...

        def TimeZoneOffset(self, timezone: str, locale: str = ...) -> int:
            """
                Gets the offset between GMT and the given timezone and locale, in minutes.
                    Args
                        ``imezone``: the timezone for which the offset to the GMT will be calculated.

                        ``locale``: the locale specifying the country for which the offset will be calculated,
                        given either in "la-CO" or "CO" formats.
                        The default value is the locale defined in the ``OfficeLocale`` property
                        of the ``Platform`` service.
                """
            ...

        def UTCDateTime(self, localdatetime: datetime.datetime, timezone: str, locale: str = ...) -> datetime.datetime:
            """
                Gets the UTC date and time considering a given local date and time in a timezone.
                    Args
                        ``localdatetime``: the local date and time in a specific timezone, expressed as a date.

                        ``timezone``: the timezone for which the ``localdatetime`` argument is given.

                        ``locale``: the locale specifying the country for which the ``localdatetime`` argument is given,
                        expressed either in "la-CO" or "CO" formats. The default value is the locale defined
                        in the ``OfficeLocale`` property of the ``Platform`` service.
                """
            ...

        def UTCNow(self, timezone: str, locale: str = ...) -> datetime.datetime:
            """
                Gets the current UTC date and time, given a timezone and a locale.
                    Args
                        ``timezone``: the timezone for which the current UTC time will be calculated.

                        ``locale``: the locale specifying the country for which the current UTC time will be calculated,
                        expressed either in "la-CO" or "CO" formats. The default value is the locale defined
                        in the ``OfficeLocale`` property of the ``Platform`` service.
                """
            ...

    # #########################################################################
    # SF_Session CLASS
    # #########################################################################
    class SF_Session(SFServices, metaclass = ...):
        """
            The Session service gathers various general-purpose methods about:
                - UNO introspection
                - the invocation of external scripts or programs
                - send mails
                - ...
            """

        # Class constants               Where to find an invoked library ?
        SCRIPTISEMBEDDED: Literal["document"]
        """ The invoked library is in the document (Basic + Python). """
        SCRIPTISAPPLICATION: Literal["application"]
        """ The invoked library is in any shared library (Basic).  """
        SCRIPTISPERSONAL: Literal["user"]
        """ The invoked library is in My Macros (Python).    """
        SCRIPTISPERSOXT: Literal["user:uno_packages"]
        """ The invoked library is in an extension installed for the current user (Python). """
        SCRIPTISSHARED: Literal["share"]
        """ The invoked library is in LibreOffice macros (Python).  """
        SCRIPTISSHAROXT: Literal["share:uno_packages"]
        """ The invoked library is in an extension installed for all users (Python).    """
        SCRIPTISOXT: Literal["uno_packages"]
        """ The invoked library is in an extension but the installation parameters are unknown (Python).    """

        @classmethod
        def ExecuteBasicScript(
            cls, scope: str = ..., script: str = ..., *args: Optional[Any]
        ) -> Any:
            """
                Execute the Basic script given its name and location and fetch its result if any.

                If the script returns nothing, which is the case of procedures defined with Sub,
                the returned value is ``None``.
                    Args
                        ``scope``: string specifying where the script is stored.

                        ``script``: string specifying the script to be called in the format
                            "library.module.method" as a case-sensitive string.

                        ``args``: the arguments to be passed to the called script.
                    Notes
                        Argument ``scope`` can be either:
                            - "document" (= ``session.SCRIPTISEMBEDDED``)
                            - "application" (= ``session.SCRIPTISAPPLICATION``).

                        About argument ``script``:
                            - The library is loaded in memory if necessary.
                            - The module must not be a class module.
                            - The method may be a ``Sub`` or a ``Function``.
                    Returns
                        The value returned by the called script.
                """
            ...

        @classmethod
        def ExecuteCalcFunction(cls, calcfunction: str, *args: Any) -> Any:
            """
                Execute a Calc function using its English name and based on the given arguments.
                If the arguments are arrays, the function is executed as an array formula.
                    Args
                        ``calcfunction``: the name of the Calc function to be called, in English.

                        ``args``: the arguments to be passed to the called Calc function.
                        Each argument must be either a string, a numeric value or an array of arrays combining
                        those types.
                    Returns
                        The value returned by the function.
                """
            ...

        @classmethod
        def ExecutePythonScript(cls, scope: str = ..., script: str = ..., *args: Any) -> Any:
            """
                Execute the Python script given its location and name, fetch its result if any.
                Result can be a single value or an array of values.

                If the script is not found, or if it returns nothing, the returned value is ``None``.
                    Args
                        ``scope``: one of the applicable constants listed. See Notes.

                        ``script``: either "library/module.py$method" or "module.py$method"
                            or "myExtension.oxt|myScript|module.py$method" as a case-sensitive string.
                    Notes
                        Arg ``scope`` can be either
                            - "document" (= ``session.SCRIPTISEMBEDDED``)
                            - "application"  (= ``session.SCRIPTISAPPLICATION``)
                            - "user"  (= ``session.SCRIPTISPERSONAL``)
                            - "user:uno_packages" (= ``session.SCRIPTISPERSOXT``)
                            - "share" (= ``session.SCRIPTISSHARED``). Default value.
                            - "share:uno_packages" (= ``session.SCRIPTISSHAROXT``)
                            - "uno_packages" (= ``session.SCRIPTISOXT``)

                        About argument ``script``:
                            - ``library``: The folder path to the Python module.
                            - ``myScript``: The folder containing the Python module.
                            - ``module.py``: The Python module.
                            - ``method``: The Python function.
                    Returns
                        The value(s) returned by the call to the script. If > 1 values, enclosed in a tuple.
                """
            ...

        def GetPDFExportOptions(self) -> DICTIONARY:
            """
                Returns the current PDF export settings defined in the ``PDF Options`` dialog, which can be accessed
                by choosing ``File - Export as - Export as PDF``.
                    Returns
                        A ``Dictionary`` object wherein each key represents an export option and the corresponding
                        value is the current PDF export setting.
                """
            ...

        def HasUnoMethod(self, unoobject: UNO, methodname: str) -> bool:
            """
                Returns ``True`` if a UNO object contains the given method.
                    Args
                        ``unoobject``: the object to inspect.

                        ``methodname``: the name of the method as a string. The search is case-sensitive.
                    Returns
                        ``False`` when the method is not found or when an argument is invalid.
                """
            ...

        def HasUnoProperty(self, unoobject: UNO, propertyname: str) -> bool:
            """
                Returns ``True`` if a UNO object contains the given property.
                    Args
                        ``unoobject``: the object to inspect.

                        ``propertyname``: the name of the property as a string. The search is case-sensitive.
                    Returns
                        ``False`` when the property is not found or when an argument is invalid.
                """
            ...

        @classmethod
        def OpenURLInBrowser(cls, url: str) -> None:
            """
                Opens a given URL in the default browser.
                    Args
                        ``url``: The URL to open in the browser.
                """
            ...

        def RunApplication(self, command: str, parameters: str) -> bool:
            """
                Executes an arbitrary system command.
                    Args
                        ``command``:  the command to execute.
                        This may be an executable file or a file which is registered with an application
                        so that the system knows what application to launch for that file. A file must be
                        expressed in the current ``filesystem.FileNaming`` notation.

                        ``parameters``: a list of space separated parameters as a single string.
                        The method does not validate the given parameters, but only passes them to the specified
                        command.
                    Returns
                        ``True`` if the command was launched successfully.
                """
            ...
        def SendMail(self,
                     recipient: str,
                     cc: str = ...,
                     bcc: str = ...,
                     subject: str = ...,
                     body: str = ...,
                     filenames: str = ...,
                     editmessage: bool = ...,
                     ) -> None:
            """
                Send a message (with or without attachments) to recipients from the user's mail client.
                The message may be edited by the user before sending or, alternatively, be sent immediately.
                    Args
                        ``recipient``: an email addresses (To recipient).

                        ``cc``: a comma-delimited list of email addresses (carbon copy). Defaults to ''.

                        ``bcc``: a comma-delimited list of email addresses (blind carbon copy). Defaults to ''.

                        ``subject``: the header of the message. Defaults to ''.

                        ``body``: the unformatted text of the message. Defaults to ''.

                        ``filenames``: a comma-separated list of filenames to attach to the mail.
                        ``filesystem.FileNaming`` naming conventions apply. Defaults to ''.

                        ``editmessage``: when ``True`` (default) the message is editable before being sent.
                """
            ...

        def SetPDFExportOptions(self, pdfoptions: DICTIONARY) -> bool:
            """
                Modifies the PDF export settings defined in the ``PDF Options`` dialog, which can be accessed
                by choosing ``File - Export as - Export as PDF``.

                The new settings can be used by the ``ExportAsPDF()`` method from the ``Document`` service.
                    Args
                        ``pdfoptions``: a ``Dictionary`` object that defines the PDF export settings to be changed.
                        Each key-value pair represents an export option and its new value.
                    Returns
                        ``True`` when successful.
                """
            ...

        def UnoMethods(self, unoobject: UNO) -> Tuple[str, ...]:
            """
                Returns a tuple of the methods callable from an UNO object. The tuple may be empty.
                    Args
                         ``unoobject``:  the object to inspect.
                """
            ...

        def UnoObjectType(self, unoobject: UNO) -> str:
            """
                Identify the UNO class of an UNO object.
                    Args
                        ``unoobject``:  the object to inspect.
                    Returns
                        ``com.sun.star. ...`` as a string or a zero-length string if the identification was not
                        successful.
                """
            ...

        def UnoProperties(self, unoobject: UNO) -> Tuple[str, ...]:
            """
                Returns a tuple of the properties callable from an UNO object. The tuple may be empty.
                    Args
                         ``unoobject``:  the object to inspect.
                """
            ...

        def WebService(self, uri: str) -> str:
            """
                Get some web content from a URI.
                    Args
                         ``uri ``: text of the web service.
                    Returns
                        The web page content of the URI.
                """
            ...
    # #########################################################################
    # SF_String CLASS
    # #########################################################################
    class SF_String(SFServices, metaclass = ...):
        """
            Focus on string manipulation, regular expressions, encodings and hashing algorithms.
            The methods implemented in Basic that are redundant with Python builtin functions
            are not duplicated.
            """

        sfCR: str
        """ Carriage return (ASCII 13). """
        sfCRLF: str
        """ Carriage return + linefeed (ASCII 13 + 10). """
        sfLF: str
        """ Linefeed (ASCII 10).    """
        sfNEWLINE: str
        """ Depending on the operating system, either Carriage return + Linefeed (ASCII 13 + 10)
        or Linefeed (ASCII 10). """
        sfTAB: str
        """ Horizontal tabulation (ASCII 09).   """

        @classmethod
        def HashStr(cls,
                    inputstr: str,
                    algorithm: Literal['MD5', 'SHA1', 'SHA224', 'SHA256', 'SHA384', 'SHA512'],
                    ) -> str:
            """
                Return a hexadecimal string representing a checksum of the given input string.
                    Args
                        ``inputstr``: the string to be hashed.

                        ``algorithm``: the hashing algorithm to use.
                    Returns
                        The requested checksum as a string. Hexadecimal digits are lower-cased.
                        A zero-length string when an error occurred.
                    Note
                        The supported hashing algorithms are:
                            - MD5
                            - SHA1
                            - SHA224
                            - SHA256
                            - SHA384
                            - SHA512
                """
            ...

        def IsADate(self,
                    inputstr: str,
                    dateformat: Literal['YYYY-MM-DD', 'DD-MM-YYYY', 'MM-DD-YYYY',
                                            'YYYY/MM/DD', 'DD/MM/YYYY', 'MM/DD/YYYY',
                                            'YYYY.MM.DD', 'DD.MM.YYYY', 'MM.DD.YYYY',
                                            'YYYY MM DD', 'DD MM YYYY', 'MM DD YYYY'] = ...,
                    ) -> bool:
            """
                Return  ``True`` if the string is a valid date respecting the given format.
                    Args
                        ``inputstr``: the input string.

                        ``dateformat``: either ``YYYY-MM-DD`` (default), ``DD-MM-YYYY`` or ``MM-DD-YYYY``.
                        The dash (-) may be replaced by a dot (.), a slash (/) or a space.
                    Returns
                        ``True`` if the string contains a valid date and there is at least one character.
                        ``False`` otherwise or if the date format is invalid.
                """
            ...

        def IsEmail(self, inputstr: str) -> bool:
            """
                Return ``True`` if the string is a valid email address.
                    Args
                        ``inputstr``: the input string.
                    Returns
                        ``True`` if the string contains an email address and there is at least one character,
                        ``False`` otherwise.
            """
            ...

        def IsFileName(self, inputstr: str, osname: Literal['WINDOWS', 'LINUX', 'MACOS', 'SOLARIS'] = ...) -> bool:
            """
                Return ``True`` if the string is a valid filename in a given operating system.
                    Args
                        ``inputstr``: the input string.

                        ``osname``: the default is the current operating system on which the script is run.
                    Returns
                        ``True`` if the string contains a valid filename and there is at least one character,

            """
            ...

        def IsIBAN(self, inputstr: str) -> bool:
            """
                Returns ``True`` if the input string is a valid International Bank Account Number.
                    Args
                        ``inputstr``: the input string.
                    Returns
                        ``True`` if the string contains a valid IBAN number. The comparison is not case-sensitive.
            """
            ...

        def IsIPv4(self, inputstr: str) -> bool:
            """
                Return ``True`` if the string is a valid IPv4 address.
                    Args
                        ``inputstr``: the input string.
                    Returns
                        ``True`` if the string contains a valid IPv4 address and there is at least one character,
                        ``False`` otherwise.
                """
            ...

        def IsLike(self, inputstr: str, pattern: str, casesensitive: bool = ...) -> bool:
            """
                Returns True if the whole input string matches a given pattern containing wildcards.
                    Args
                        ``inputstr``: the input string.

                        ``pattern``:  the pattern as a string. The ``?`` represents any single character.
                        The ``*`` represents zero, one, or multiple characters.

                        ``casesensitive``: defaults to ``False``.
                    Returns
                        ``True`` if a match is found. Zero-length input or pattern strings always return ``False``.
                """
            ...

        def IsSheetName(self, inputstr: str) -> bool:
            """
                Return ``True`` if the input string can serve as a valid Calc sheet name.

                The sheet name must not contain the characters ``[ ] * ? : / \\``
                or the character ``'`` (apostrophe) as first or last character.
                    Args
                        ``inputstr``: the input string.
                    Returns
                        ``True`` if the string is validated as a potential Calc sheet name, ``False`` otherwise.
                """
            ...

        def IsUrl(self, inputstr: str) -> bool:
            """
                Return ``True`` if the string is a valid absolute URL (Uniform Resource Locator).
                    Args
                        ``inputstr``: the input string.
                    Returns
                        ``True`` if the string contains a URL and there is at least one character, ``False`` otherwise.
            """
            ...

        def SplitNotQuoted(self,
                           inputstr: str,
                           delimiter: str = ...,
                           occurrences: int = ...,
                           quotechar: Literal["'", '"'] = ...,
                           ) -> Tuple[str, ...]:
            """
                Split a string on ``delimiter`` into an array. If ``delimiter`` is part of a quoted (sub)string,
                it is ignored. Use case: parsing of csv-like records containing quoted strings.
                    Args
                        ``inputstr``: the input string.

                        ``delimiter``: a string of one or more characters that is used to delimit the input string.
                        Defaults to ' ' (space).

                        ``occurrences``: the maximum number of substrings to return (Default = 0, meaning no limit).

                        ``quotechar``: the quoting character, either ``"`` (default) or ``'``.
                    Returns
                        A tuple whose items are chunks of the input string, The ``delimiter`` is not included.
                """
            ...

        def Wrap(self, inputstr: str, width: int = ..., tabsize: int = ...) -> Tuple[str, ...]:
            """
            Wraps every single paragraph in text (a string) so every line is at most ``width`` characters long.
                Args
                    ``inputstr``: the input string.

                    ``width``: the maximum number of characters in each line. Defaults to 70.

                    ``tabsize``: before wrapping the text, the existing TAB (ASCII 09) characters are replaced with
                    spaces. ``TabSize`` defines the TAB positions at
                    ``TabSize + 1, 2 * TabSize + 1 , ... N * TabSize + 1``. Defaults to 8.
                Returns
                    A tuple of output lines, without final newlines except the pre-existing line-breaks.
                    Tabs are expanded. Symbolic line breaks are replaced by their hard equivalents.
                    If the wrapped output has no content, the returned tuple is empty.
            """
            ...

    # #########################################################################
    # SF_TextStream CLASS
    # #########################################################################
    class SF_TextStream(SFServices):
        """
            The ``TextStream`` service is used to sequentially read from and write to files opened or created
            using the ScriptForge.FileSystem service.
            """

        AtEndOfStream: bool
        """ Used in read mode. A ``True`` value indicates that the end of the file has been reached.
        A test using this property should precede calls to the ``ReadLine()`` method.   """
        Encoding: str
        """ The character set to be used. The default encoding is "UTF-8".  """
        FileName: FILE
        """ Returns the name of the current file either in URL format or in the native operating system's format,
        depending on the current value of the ``FileNaming`` property of the ``filesystem`` service.    """
        IOMode: Literal['READ', 'WRITE', 'APPEND']
        """ The input/output mode.  """
        Line: int
        """ The number of lines read or written so far. """
        NewLine: str
        """ Sets or returns the current delimiter to be inserted between two successive written lines.
        The default value is the native line delimiter in the current operating system. """

        def CloseFile(self) -> bool:
            """
                Empties the output buffer if relevant. Closes the actual input or output stream.
                    Returns
                        ``True`` if the closure was successful.
                """
            ...

        def ReadAll(self) -> str:
            """
                Returns all the remaining lines in the text stream as one string. Line breaks are NOT removed.
                The resulting string can be split in lines either by using the usual ``split()`` builtin function.
                    Returns
                        The read lines. The string may be empty.
                    Note
                        The ``Line`` property is incremented only by 1.
                """
            ...

        def ReadLine(self) -> str:
            """
                Returns the next line in the text stream as a string. Line breaks are removed.
                    Returns
                        The read line. The string may be empty.
                """
            ...

        def SkipLine(self) -> None:
            """
                Skips the next line when reading a ``TextStream`` file. This method can result in ``AtEndOfStream``
                being set to ``True``.
                """
            ...

        def WriteBlankLines(self, lines: int) -> None:
            """
                Writes a number of empty lines in the output stream.
                    Args
                        ``lines``: the number of lines to write.
                """
            ...

        def WriteLine(self, line: str) -> None:
            """
                Writes the given line to the output stream. A newline is inserted if relevant.
                    Args
                        ``line``: the line to write, may be the empty string.
                """
            ...

    # #########################################################################
    # SF_Timer CLASS
    # #########################################################################
    class SF_Timer(SFServices):
        """
            The ``Timer`` service measures the amount of time it takes to run user scripts.

            A ``Timer`` measures durations. It can be:
                - Started, to indicate when to start measuring time.
                - Suspended, to pause measuring running time.
                - Resumed, to continue tracking running time after the Timer has been suspended.
                - Restarted, which will cancel previous measurements and start the Timer at zero.
            """

        Duration: float
        """ The actual running time elapsed since start or between start and stop (not considering suspended time). """
        IsStarted: bool
        """ ``True`` when timer is started or suspended.    """
        IsSuspended: bool
        """ ``True`` when timer is started and suspended.   """
        SuspendDuration: float
        """ The actual time elapsed while suspended since start or between start and stop.  """
        TotalDuration: float
        """ The actual time elapsed since start or between start and stop (including suspensions and running time).
        ``TotalDuration = Duration + SuspendDuration``. """

        def Continue(self) -> bool:
            """
                Continue/Resume a suspended timer.
                    Returns
                        ``True`` except if the timer is not in suspended state.
                """
            ...

        def Restart(self) -> bool:
            """
                Terminate the timer and restart a new clean timer.
                    Returns
                        ``True`` except if the timer is inactive.
                """
            ...

        def Start(self) -> bool:
            """
                Start a new clean timer.
                    Returns
                        ``True`` except if the timer is already started.
                """
            ...

        def Suspend(self) -> bool:
            """
                Suspend a running timer.
                    Returns
                        ``True`` except if the timer is not started or already suspended.
                """
            ...

        def Terminate(self) -> bool:
            """
                Terminate a running timer.
                    Returns
                        ``True`` except if the timer is neither started nor suspended.
                """
            ...

    # #########################################################################
    # SF_UI CLASS
    # #########################################################################
    class SF_UI(SFServices, metaclass = ...):
        """
            Class for the identification and the manipulation of the different windows composing the whole
            ``LibreOffice`` application:
                - Windows selection
                - Windows moving and resizing
                - Statusbar settings
                - Creation of new windows
                - Access to the underlying "documents"

            Windows can be designated using different ``WindowNames``:
                - a full path and file name
                - the last component of the full file name or even only the last component without its suffix
                - the title of the window
                - for new documents, something like "Untitled 1"
                - one of the special windows ``BASICIDE`` and ``WELCOMESCREEN``
            A ``WindowName`` is case-sensitive.
            """

        MACROEXECALWAYS: Literal[2]
        """ Macros are always executed.  """
        MACROEXECNEVER: Literal[1]
        """ Macros are never executed.  """
        MACROEXECNORMAL: Literal[0]
        """ Macro execution depends on user settings.   """

        ActiveWindow: str
        """ A valid and unique WindowName for the currently active window. When the window cannot be identified,
        a zero-length string is returned.   """

        Height: int
        """ The height of the active window in pixels.  """
        Width: int
        """ The width of the active window in pixels.   """

        X: int
        """ The X coordinate of the active window, which is the distance to the left edge of the screen in pixels.  """

        Y: int
        """ The Y coordinate of the active window, which is the distance to the top edge of the screen in pixels.
        This value does not consider window decorations added by your operating system, so even when the window
        is maximized this value may not be zero.    """

        def Activate(self, windowname: str = ...) -> bool:
            """
                Make the specified window active.
                    Args
                        ``windowname``: see definitions at ``SF_UI`` class level.
                    Returns
                        ``True`` if the given window is found and can be activated.
                        There is no change in the actual user interface if no window matches the selection.
                """
            ...

        def CreateBaseDocument(self,
                               filename: str,
                               embeddeddatabase: Literal['HSQLDB', 'FIREBIRD', 'CALC'] = ...,
                               registrationname: str = ...,
                               calcfilename: str = ...,
                               ) -> BASE:
            """
                Create a new LibreOffice Base document embedding an empty database of the given type.
                    Args
                        ``filename``: identifies the file to create. It must follow the ``FileSystem.FileNaming``
                        notation. If the file already exists, it is overwritten without warning.

                        ``embeddeddatabase``: either ``HSQLDB`` or ``FIREBIRD`` or ``CALC``. Defaults to ``HSQLDB``.

                        ``registrationname``: the name used to store the new database in the databases register.
                        If "" (default), no registration takes place. If the name already exists it is overwritten
                        without warning.

                        ``calcfilename``: only when ``embeddeddatabase`` = ``CALC``, the name of the file containing
                        the tables as Calc sheets. The name of the file must be given in ``FileSystem.FileNaming``
                        notation. The file must exist.
                    Returns
                        A ``Base`` service instance.
                """
            ...

        def CreateDocument(self,
                           documenttype: Literal['', 'calc', 'draw', 'impress', 'math', 'writer',
                                                'Calc', 'Draw', 'Impress', 'Math', 'Writer',
                                                'CALC', 'DRAW', 'IMPRESS', 'MATH', 'WRITER'] = ...,
                           templatefile: FILE = ...,
                           hidden: bool = ...
                           ) -> Union[DOCUMENT, CALC, WRITER]:
            """
                Create a new ``LibreOffice`` document of a given type or based on a given template.
                    Args
                        ``documenttype``: Calc, Writer, etc. If absent, a ``templatefile`` must be given.

                        ``templatefile``: the full ``FileName`` of the template to build the new document on.
                        If the file does not exist, the argument is ignored.
                        The ``FileSystem`` service provides the ``TemplatesFolder`` and ``UserTemplatesFolder``
                        properties to help to build the argument. Defaults to ''.

                        ``hidden``: if ``True``, open in the background. Defaults to ``False``.
                        To use with caution: activation or closure can only happen programmatically.
                    Returns
                        A ``Document`` object or one of its subclasses.
                    Note
                        Use the ``CreateBaseDocument()`` method to create a new ``Base`` document.
                """
            ...

        def Documents(self) -> Tuple[str, ...]:
            """
                Returns the list of the currently open documents. Special windows are ignored.
                    Returns
                        An array either of file names (in ``FileSystem.FileNaming`` notation) or of window titles
                        for unsaved documents.
                """
            ...

        def GetDocument(
            self,
            windowname: Union[str, XComponent, XOfficeDatabaseDocument] = ...
        ) -> Union[BASE, CALC, DOCUMENT, FORMDOCUMENT, WRITER]:
            """
                Returns a ``Document`` object referring to the active window or the given window.
                    Args
                        ``windowname``: when a string, see definitions in ``SF_UI``. If absent the active window is
                        considered. When an object, must be a UNO object of types ``XComponent``
                        or ``XOfficeDatabaseDocument``.
                    Returns
                        A ``Document`` object or one of its subclasses.

                """
            ...

        def Maximize(self, windowname: str = ...) -> None:
            """
                Maximizes the active window or the given window.
                    Args
                        ``windowname``: see definitions in ``SF_UI``. If absent the active window is considered.
                """
            ...

        def Minimize(self, windowname: str = ...) -> None:
            """
                Minimizes the active window or the given window.
                    Args
                        ``windowname``: see definitions in ``SF_UI``. If absent the active window is considered.
                """
            ...

        def OpenBaseDocument(self,
                             filename: FILE = ...,
                             registrationname: str = ...,
                             macroexecution: Literal[0, 1, 2] = ...,
                             ) -> BASE:
            """
                Open an existing LibreOffice Base document and return a SFDocuments.Base object.
                    Args
                        ``filename``: identifies the file to open.
                        It must follow the ``FileSystem.FileNaming`` notation. Defaults to ''.

                        ``registrationname``: the name of a registered database.
                        It is ignored if FileName <> "". Defaults to ''.

                        ``macroexecution``: one of the MACROEXECxxx constants. Defaults to ``MACROEXECNORMAL``.
                    Returns
                        A ``Base`` object.
                """
            ...

        def OpenDocument(self,
                         filename: FILE,
                         password: str = ...,
                         readonly: bool = ...,
                         hidden: bool = ...,
                         macroexecution: Literal[0, 1, 2] = ...,
                         filtername: str = ...,
                         filteroptions: str = ...,
                         ) -> Union[DOCUMENT, CALC, WRITER]:
            """
                Open an existing LibreOffice document with the given options.
                    Args
                        ``filename``: identifies the file to open. It must follow the ``FileSystem.FileNaming``
                        notation.

                        ``password``: to use when the document is protected. If wrong or absent while the document
                        is protected, the user will be prompted to enter a password.

                        ``readonly``: defaults to ``False``.

                        ``hidden``: if ``True``, open in the background. Defaults to ``False``.
                        ``True`` to use with caution: activation or closure can only happen programmatically.

                        ``macroexecution``: one of the MACROEXECxxx constants. Defaults to ``MACROEXECNORMAL``.

                        ``filtername``: the name of a filter that should be used for loading the document.
                        If present, the filter must exist. Defaults to ''.

                        ``filteroptions``: an optional string of options associated with the filter.
                    Returns
                        A ``Document`` object or one of its subclasses, or
                        None if the opening failed, including when due to a user decision.
                """
            ...

        def Resize(self, left: int = ..., top: int = ..., width: int = ..., height: int = ...) -> None:
            """
                Resizes and/or moves the active window. Negative or absent arguments are ignored.
                If the window was minimized or without arguments, it is restored.
                    Args
                        ``left``: distance from left edge of screen.

                        ``top``: distance from top of screen.

                        ``width``: width of the window.

                        ``height``: height of the window.
                    """
            ...

        def RunCommand(self, command: str, *args: Any, **kwargs: Any) -> None:
            """
                Runs a UNO command on the current window. Commands can be run with or without arguments.
                Arguments are not validated before running the command. If the command or its arguments are invalid,
                then nothing will happen.
                    Args
                        ``command``: case-sensitive string containing the UNO command name.
                        The inclusion of the prefix ".uno:" in the command is optional.
                        The command itself is not checked for correctness.

                        ``args, kwargs``: use either pairs of ("name", value) positional arguments
                        or pairs of name = value keyword arguments.
                """
            ...

        def SetStatusbar(self, text: str = ..., percentage: Union[int, float] = ...) -> None:
            """
                Display a text and a progressbar in the status bar of the active window.
                Any subsequent calls in the same macro run refer to the same status bar of the same window,
                even if the window is not active anymore.
                A call without arguments resets the status bar to its normal state.
                    Args
                        ``text``: the optional text to be displayed before the progress bar. Defaults to ''.

                        ``percentage``: the optional degree of progress between 0 and 100.
                """
            ...
        def ShowProgressBar(self, title: str = ..., text: str = ..., percentage: Union[int, float] = ...) -> None:
            """
                Display a non-modal dialog box. Specify its title, an explicatory text and the progress
                on a progressbar. A call without arguments erases the progress bar dialog.
                The box will anyway vanish at the end of the macro run.
                    Args
                        ``title``: the title appearing on top of the dialog box (Default = ``ScriptForge``).

                        ``text``: the optional text to be displayed above the progress bar. Defaults to ''.

                        ``percentage``: the degree of progress between 0 and 100.
                """
            ...

        def WindowExists(self, windowname: str) -> bool:
            """
                Returns ``True`` if the specified window exists.
                    Args
                        ``windowname``: see the definitions in ``SF_UI``.
                    Returns
                        ``True`` if the given window is found.
                """
            ...


# #####################################################################################################################
#                       SFDatabases CLASS    (alias of SFDatabases Basic library)                                   ###
# #####################################################################################################################
class SFDatabases:
    """
        The SFDatabases class manages databases embedded in or connected to Base documents.
        The  associated services include the ``Database``, ``Dataset`` and ``Datasheet`` services.
        """

    # #########################################################################
    # SF_Database CLASS
    # #########################################################################
    class SF_Database(SFServices):
        """
            Each instance of the current class represents a single database, stored in or connected
            to a ``Base`` document, with essentially its tables, queries and data.

            The exchanges with the database are done in SQL only.
            To make them more readable, use optionally square brackets to surround table/query/field names
            instead of the (RDBMS-dependent) normal surrounding character.

            ``SQL`` statements may be run in direct or indirect modes. In direct mode the statement is transferred
            literally without syntax checking nor review to the database engine.
            """

        Queries: Tuple[str, ...]
        """ The list of stored queries. """
        Tables: Tuple[str, ...]
        """ The list of stored tables.  """
        XConnection: UNO
        """ The ``UNO`` object representing the current connection (``com.sun.star.sdbc.XConnection``). """
        XMetaData: UNO
        """ The ``UNO`` object representing the metadata describing the database system attributes
        (``com.sun.star.sdbc.XDatabaseMetaData``).  """

        def CloseDatabase(self) -> None:
            """
                Close the current database connection.
                """
            ...

        def Commit(self) -> None:
            """
                Commits all updates done since the previous ``Commit`` or ``Rollback`` call.
                This method is ignored if commits are done automatically after each SQL statement,
                i.e. the database is set to the default auto-commit mode.
                """
            ...

        def CreateDataset(self, sqlcommand: SQL_SELECT, directsql: bool = ..., filter: str = ..., orderby: str = ...
        ) -> Optional[DATASET]:
            """
                Creates a Dataset service instance based on a table, query or ``SQL SELECT`` statement.
                    Args
                        ``sqlcommand``: a table name, a query name or a valid ``SQL SELECT`` statement.
                        Identifiers may be enclosed with square brackets. This argument is case-sensitive.

                        ``directsql``: set this argument to ``True`` to send the statement directly to the database
                        engine without preprocessing by LibreOffice (Default = ``False``).

                        ``filter``: specifies the condition that records must match to be included in the returned
                        dataset. This argument is expressed as a ``SQL WHERE`` statement without the ``WHERE`` keyword.

                        ``orderby``: specifies the ordering of the dataset as a ``SQL ORDER BY`` statement
                        without the ``ORDER BY`` keyword.
                    Returns
                        A ``Dataset`` service instance.
                """
            ...

        def DAvg(self, expression: str, tablename: str, criteria: str = ...) -> Optional[float, int]:
            """
                Compute the aggregate function ``AVG()`` on a  field or expression belonging to a table
                filtered by a ``WHERE``-clause.
                    Args
                        ``expression``: an ``SQL`` expression.

                        ``tablename``: the name of a table.

                        ``criteria``: an optional ``WHERE`` clause without the word ``WHERE``.
                    Returns
                        int | float | None
                """
            ...

        def DCount(self, expression: str, tablename: str, criteria: str = ...) -> Optional[int]:
            """
                Compute the aggregate function ``COUNT()`` on a  field or expression belonging to a table
                filtered by a ``WHERE``-clause.
                    Args
                        ``expression``: an ``SQL`` expression.

                        ``tablename``: the name of a table.

                        ``criteria``: an optional ``WHERE`` clause without the word ``WHERE``.
                    Returns
                        int | float | None
                """
            ...

        def DLookup(self, expression: str, tablename: str, criteria: str = ..., orderclause: str = ...) -> Any:
            """
                Compute a ``SQL`` expression on a single record returned by a ``WHERE`` clause defined by the
                ``criteria`` parameter.

                If the query returns multiple records, only the first one is considered.
                Use the ``orderclause`` parameter to determine how query results are sorted.
                    Args
                        ``expression``: an ``SQL`` expression in which the field names are surrounded with
                        square brackets.

                        ``tablename``: the name of a table (without square brackets).

                        ``criteria``: an optional  ``WHERE`` clause without the word ``WHERE``, in which field names
                        are surrounded with square brackets.

                        ``orderclause``: an optional ``ORDER BY`` clause without the ``ORDER BY`` keywords.
                        Field names should be surrounded with square brackets.
                    Returns
                        The found value or None.
                """
            ...

        def DMax(self, expression: str, tablename: str, criteria: str = ...) -> Optional[float, int]:
            """
                Compute the aggregate function ``MAX()`` on a  field or expression belonging to a table
                filtered by a ``WHERE``-clause.
                    Args
                        ``expression``: an ``SQL`` expression.

                        ``tablename``: the name of a table.

                        ``criteria``: an optional ``WHERE`` clause without the word ``WHERE``.
                    Returns
                        int | float | None
                """
            ...

        def DMin(self, expression: str, tablename: str, criteria: str = ...) -> Optional[float, int]:
            """
                Compute the aggregate function ``MIN()`` on a  field or expression belonging to a table
                filtered by a ``WHERE``-clause.
                    Args
                        ``expression``: an ``SQL`` expression.

                        ``tablename``: the name of a table.

                        ``criteria``: an optional ``WHERE`` clause without the word ``WHERE``.
                    Returns
                        int | float | None
                """
            ...

        def DSum(self, expression: str, tablename: str, criteria: str = ...) -> Optional[float, int]:
            """
                Compute the aggregate function ``SUM()`` on a  field or expression belonging to a table
                filtered by a ``WHERE``-clause.
                    Args
                        ``expression``: an ``SQL`` expression.

                        ``tablename``: the name of a table.

                        ``criteria``: an optional ``WHERE`` clause without the word ``WHERE``.
                    Returns
                        int | float | None
                """
            ...

        def GetRows(self,
                    sqlcommand: SQL_SELECT,
                    directsql: bool = ...,
                    header: bool = ...,
                    maxrows: int = ...,
                    ) -> MATRIX:
            """
                Return the content of a table, a query or a ``SELECT SQL`` statement as a list of lists.
                The first index corresponds to the individual records and the second index refers to the record fields.
                    Args
                        ``sqlcommand``: a table name, a query name or a ``SELECT SQL`` statement.

                        ``directsql``: when ``True``, no syntax conversion is done by LibreOffice.
                        Ignored when ``sqlommand`` is a table or a query name. Defaults to ``False``.

                        ``header``: when ``True``, a header row is inserted on the top of the list with the
                        column names. Defaults to ``False``.

                        ``maxrows``: the maximum number of returned rows. If absent, all records are returned.
                    Returns
                        The returned list will be empty if no rows are returned and the column headers are not required.
                """
            ...

        def OpenFormDocument(self, formdocument: str) -> Optional[FORMDOCUMENT]:
            """
                Open the ``FormDocument`` given by its hierarchical name in normal mode.
                If the form document is already open, the form document is made active.
                    Args
                        ``formdocument``: a valid form document name as a case-sensitive string.
                            When hierarchical, the hierarchy must be rendered with forward slashes ("/").
                    Returns
                        A ``FormDocument`` instance or ``None``.
                """
            ...

        def OpenQuery(self, queryname: str) -> Optional[DATASHEET]:
            """
                Opens the Data View window of the specified query.
                    Args
                        ``queryname``: the name of an existing query as a case-sensitive string.
                    Returns
                        An instance of the ``Datasheet`` service.
                        If the query could not be opened, then ``None`` is returned.
                """
            ...

        def OpenSql(self, sql: SQL_SELECT, directsql: bool = ...) -> Optional[DATASHEET]:
            """
                Runs a ``SQL SELECT`` command, opens a Data View window with the results.
                    Args
                        sql: a string containing a valid ``SQL SELECT`` statement.
                        Identifiers may be enclosed by square brackets.

                        directsql: when ``True``, the ``SQL`` command is sent to the database engine
                        without pre-analysis. Defaults to ``False``.
                    Returns
                        An instance of the ``Datasheet`` service.
                        If the query could not be opened, then ``None`` is returned.
                """
            ...

        def OpenTable(self, tablename: str) -> Optional[DATASHEET]:
            """
                Opens the Data View window of the specified table.
                    Args
                        ``tablename``: the name of an existing table as a case-sensitive string.
                    Returns
                        An instance of the ``Datasheet`` service.
                        If the table could not be opened, then ``None`` is returned.
                """
            ...

        def Rollback(self) -> None:
            """
                Cancels all updates made to the database since the previous ``Commit`` or ``Rollback`` call.
                This method is ignored if commits are done automatically after each SQL statement,
                i.e. if the database is set to the default auto-commit mode.
                """
            ...

        def RunSql(self, sqlcommand: SQL_ACTION, directsql: bool = ...) -> bool:
            """
                Execute an action query (table creation, record insertion, ...) or ``SQL`` statement on
                the current database.
                    Args
                        ``sqlcommand``: a query name (without square brackets) or an ``SQL`` statement.

                        ``directsql``: when ``True``, no syntax conversion is done by LibreOffice.
                        Ignored when ``sqlcommand`` is a query name. Defaults to ``False``.
                    Returns
                        ``True`` when the command ran successfully.
                """
            ...

        def SetTransactionMode(self, transactionmode: Literal[0, 1, 2, 4, 8] = ...) -> bool:
            """
                Defines the level of isolation in database transactions.

                By default, databases manage transactions in auto-commit mode, which means that a ``Commit()``
                is automatically performed after every SQL statement.

                Use this method to manually determine the isolation level of transactions.
                When a transaction mode other than ``NONE (0)`` is set, the script has to explicitly
                call the ``Commit()`` method to apply the changes to the database.

                    Args
                        ``tranactionmode``: specifies the transaction mode.
                        This argument must be one of the constants defined in ``com.sun.star.sdbc.TransactionIsolation``
                        (Default = ``NONE``)
                    Returns
                        ``True`` when successful.
                """
            ...

    # #########################################################################
    # SF_Dataset CLASS
    # #########################################################################
    class SF_Dataset(SFServices):
        """
            A dataset represents a set of tabular data produced by a database.

            In the user interface of LibreOffice a dataset corresponds with the data
            displayed in a form or a data sheet (table, query).

            To use datasets, the database instance must exist but the Base document may not be open.

            With this service it is possible to:
                - Navigate through and access the data in a dataset.
                - Update, insert and remove records in a dataset.
            """

        BOF: bool
        """ Returns ``True`` if the current record position is before the first record in the dataset,
        otherwise returns ``False``. Set this property to ``True`` to move the cursor to the beginning of the dataset.
        Setting this property to ``False`` is ignored.  """
        DefaultValues: Dict
        """ Returns a ``dict`` with the default values used for each field in the dataset.
        The fields or columns in the dataset are the keys in the dictionary. The database field types are converted
        to their corresponding Python data types. When the field type is undefined, the default value is None.  """
        EOF: bool
        """ Returns ``True`` if the current record position is after the last record in the dataset,
        otherwise returns ``False``. Set this property to ``True`` to move the cursor to the end of the dataset.
        Setting this property to ``False`` is ignored.  """
        Fields: Tuple[str, ...]
        """ Returns a list containing the names of all fields in the dataset.   """
        Filter: str
        """ Returns the filter applied in addition to the eventual ``WHERE`` clause(s) in the initial ``SQL`` statement.
        This property is expressed as a ``WHERE`` clause without the ``WHERE`` keyword. """
        OrderBy: str
        """ Returns the ordering clause that replaces the eventual ``ORDER BY`` clause present in the initial
        ``SQL`` statement. This property is expressed as a ``ORDER BY`` clause without the ``ORDER BY`` keywords.   """
        ParentDatabase: DATABASE
        """ Returns the ``Database`` instance corresponding to the parent database of the current dataset.  """
        RowCount: int
        """ Returns the exact number of records in the dataset. Note that the evaluation of this property
        implies browsing the whole dataset, which may be costly depending on the dataset size.  """
        RowNumber: int
        """ Returns the number of the current record starting at 1. Returns 0 if this property is unknown. """
        Source: str
        """ Returns the source of the dataset. It can be either a table name, a query name or a ``SQL`` statement.  """
        SourceType: str
        """ Returns the source of the dataset. It can be one of the following string values:
        ``TABLE``, ``QUERY`` or ``SQL``.    """
        UpdatableFields: List
        """ Returns a ``list`` containing the names of all fields in the dataset that are updatable.    """
        Values: Dict
        """ Returns a ``dict`` containing the pairs (field name: value) of the current record in the dataset.   """
        XRowSet: UNO
        """ Returns the ``com.sun.star.sdb.RowSet`` ``UNO`` object representing the dataset.    """

        def CloseDataset(self) -> bool:
            """
                Closes the current Dataset.
                    Returns
                        ``True`` when successful.
                """
            ...

        def CreateDataset(self, filter:str = ..., orderby: str = ...) -> DATASET:
            """
                Returns a ``Dataset`` service instance derived from the actual dataset by applying
                the specified ``filter`` and ``orderby`` arguments.
                    Args
                        ``filter``: specifies the condition that records must match to be included in the
                        returned dataset. This argument is expressed as a ``SQL WHERE`` clause
                        without the ``WHERE`` keyword. If this argument is not specified, then the filter
                        used in the current dataset is applied, otherwise the current filter is replaced
                        by this argument.

                        ``orderby``: specifies the ordering of the dataset as a ``SQL ORDER BY`` clause
                        without the ``ORDER BY`` keywords. If this argument is not specified, then
                        the sorting order used in the current dataset is applied, otherwise
                        the current sorting order is replaced by this argument.
                    Returns
                        A new ``Dataset`` service instance.
                """
            ...

        def Delete(self) -> bool:
            """
                Deletes the current record from the dataset.

                After this operation the cursor is positioned at the record immediately after the deleted record.
                When the deleted record is the last in the dataset, then the cursor is positioned after it
                and the property ``EOF`` returns ``True``.
                    Returns
                        ``True`` when successful.
                """
            ...

        def ExportValueToFile(self, fieldname: str, filename: FILE, overwrite: bool = ...) -> bool:
            """
                Exports the value of a binary field of the current record to the specified file.
                If the specified field is not binary or if it contains no data, then the output file is not created.
                    Args
                        ``fieldname``: the name of the binary field to be exported, as a case-sensitive string.

                        ``filename``: the complete path to the file to be created using the notation defined
                        in the ``FileSystem.FileNaming`` property.

                        ``overwrite``: set this argument to ``True`` to allow the destination file
                        to be overwritten (Default = ``False``).
                    Returns
                        ``True`` when successful.
                """
            ...

        def GetRows(self, header: bool = ..., maxrows: int = ...) -> Optional[MATRIX]:
            """
                Return the content of the dataset as a list of lists.
                The first index corresponds to the individual records and the second index refers to the record fields.
                    Args
                        ``header``: when ``True``, a header row is inserted on the top of the list with the
                        column names. Defaults to ``False``.

                        ``maxrows``: the maximum number of returned rows. If absent, all records are returned.
                    Returns
                        The returned list will be empty if no rows are returned and the column headers are not required.
                """
            ...

        def GetValue(self, fieldname: str) -> Any:
            """
                Returns the value of the specified field from the current record of the dataset.
                    Args
                        ``fieldname``: the name of the field to be returned, as a case-sensitive string.
                    Returns
                        If the specified field is binary, then its length is returned.
                """
            ...

        def Insert(self, *args, **kwargs: Dict) -> int:
            """
                Inserts a new record at the end of the dataset and initialize its fields with the specified values.
                Updatable fields with unspecified values are initialized with their default values.
                    Args
                        Next variants are allowed:
                            - ``newid = dataset.Insert({"Name": "John", "Age": 50, "City": "Chicago"})``
                            - ``newid = dataset.Insert("Name", "John", "Age", 50, "City", "Chicago")``
                            - ``newid = dataset.Insert(Name = "John", Age = 50, City = "Chicago")``
                    Returns
                        If the primary key of the dataset is an ``auto value``, then this method returns
                        the primary key value of the new record. Otherwise, the method will return 0 (when successful)
                        or -1 (when not successful).
                """
            ...

        def MoveFirst(self) -> bool:
            """
                Moves the dataset cursor to the first record. Deleted records are ignored by this method.
                    Returns
                        ``True`` when successful.
                """
            ...

        def MoveLast(self) -> bool:
            """
                Moves the dataset cursor to the last record. Deleted records are ignored by this method.
                    Returns
                        ``True`` when successful.
                """
            ...

        def MoveNext(self, offset: int = ...) -> bool:
            """
                Moves the dataset cursor forward by a given number of records.
                Deleted records are ignored by this method.
                    Args
                        ``offset``: the number of records by which the cursor shall be moved forward.
                        This argument may be a negative value (Default = +1).
                    Returns
                        ``True`` when successful.
                """
            ...

        def MovePrevious(self, offset: int = 1) -> bool:
            """
                Moves the dataset cursor backward by a given number of records.
                Deleted records are ignored by this method.
                    Args
                        ``offset``: the number of records by which the cursor shall be moved backward.
                        This argument may be a negative value (Default = +1).
                    Returns
                        ``True`` when successful.
                """
            ...

        def Reload(self, filter: str = ..., orderby: str = ...) -> bool:
            """
                Reloads the dataset from the database. The properties  ``Filter`` and  ``OrderBy`` may be defined
                when calling this method.

                Reloading the dataset is useful when records have been inserted to or deleted from the database.
                Note that the methods ``CreateDataset`` and ``Reload`` perform similar functions,
                however ``Reload`` reuses the same Dataset class instance.
                    Args
                        ``filter``: specifies the condition that records must match to be included in the
                        returned dataset. This argument is expressed as a ``SQL WHERE`` clause
                        without the ``WHERE`` keyword. If this argument is not specified, then the filter
                        used in the current dataset is applied, otherwise the current filter is replaced
                        by this argument.

                        ``orderby``: specifies the ordering of the dataset as a ``SQL ORDER BY`` clause
                        without the ``ORDER BY`` keywords. If this argument is not specified, then
                        the sorting order used in the current dataset is applied, otherwise
                        the current sorting order is replaced by this argument.
                    Returns
                        ``True`` when successful.
                """
            ...

        def Update(self, *args, **kwargs) -> bool:
            """
                Updates the values of the specified fields in the current record.
                    Args
                        Next variants are allowed:
                            - ``newid = dataset.Update({"Age": 51, "City": "New York"})``
                            - ``newid = dataset.Update("Age", 51, "City", "New York")``
                            - ``newid = dataset.Update(Age = 51, City = "New York")``
                    Returns
                        ``True`` when successful.
                """
            ...

    # #########################################################################
    # SF_Datasheet CLASS
    # #########################################################################
    class SF_Datasheet(SFServices):
        """
            The Datasheet service allows to visualize the contents of database tables as well as the results
            of queries and ``SQL`` statements using ``Base's Data View``.

            Additionally, this service allows to:
                - add custom menus to the data view
                - access values in specific positions of the data view
                - position the cursor in a specific cell of the data view

            The ``Base`` document owning the data may or may not be opened.
            """

        ColumnHeaders: Tuple[str, ...]
        """ Returns a list with the names of the column headers in the datasheet.   """
        CurrentColumn: str
        """ Returns the currently selected column name. """
        CurrentRow: int
        """ Returns the number of the currently selected row, starting at 1.    """
        DatabaseFileName: FILE
        """ Returns the file name of the ``Base`` file in ``SF_FileSystem.FileNaming`` format.  """
        Filter: str
        """ Specifies a filter to be applied to the datasheet expressed as the ``WHERE`` clause of a ``SQL`` query
        without the ``WHERE`` keyword. If an empty string is specified then the active Filter is removed. """
        LastRow: int
        """ Returns the number of rows in the datasheet.    """
        OrderBy: str
        """ Specifies the order in which records are shown expressed as the ``ORDER BY`` clause of a ``SQL`` query
        without the ``ORDER BY`` keyword. If an empty string is specified then the active ``OrderBy`` is removed.   """
        ParentDatabase: DATABASE
        """ Returns the ``Database`` instance corresponding to the parent database of the current datasheet.  """
        Source: SQL_SELECT
        """ Returns the source of the datasheet. It can be either a table name, a query name
        or a ``SQL`` statement.  """
        SourceType: str
        """ Returns the type of the data source of the datasheet. It can be one of the following string values:
        ``TABLE``, ``QUERY`` or ``SQL``.    """

        XComponent: UNO
        """ Returns the ``com.sun.star.lang.XComponent`` ``UNO`` object representing the datasheet.    """

        XControlModel: UNO
        """ Returns the ``com.sun.star.awt.XControl`` ``UNO`` object representing the datasheet.    """

        XTabControllerModel: UNO
        """ Returns the ``com.sun.star.awt.XTabControllerModel `` ``UNO`` object representing the datasheet.    """

        def Activate(self) -> None:
            """
                Brings to front the data view window referred to by the ``Datasheet`` instance.
                """
            ...

        def CloseDatasheet(self) -> None:
            """
                Closes the data view window referred to by the ``Datasheet`` instance.
                """
            ...

        def CreateMenu(self, menuheader: str, before: Union[str, int] = ..., submenuchar: str = ...) -> object:
            """
                Creates a new menu entry in the data view window and returns a ``SFWidgets.Menu`` service instance,
                with which menu items can be programmatically added.
                    Args
                        ``menuheader``: the name of the new menu.
                        ``before``: this argument can be either the name of an existing menu entry before
                        which the new menu will be placed or a number expressing the position of the new menu.
                        If this argument is left blank the new menu is placed as the last entry.

                        ``submenuchar``: the delimiter used in menu trees . Defaults to ``">"``.
                    Returns
                        A ``Menu`` service instance.
                    Note
                        Menus added using the ``CreateMenu`` method are lost as soon as the data view window is closed.
                """
            ...

        def GetText(self, column: Union[str, int] = ...) -> str:
            """
                Returns the text in a given column of the current row.
                    Args
                        ``column``: the name of the column as a string or the column position (starting at ``1``).
                        If a position greater than the number of columns is given, the last column is returned.
                    Note
                        This method does not change the position of the cursor in the data view window.
                    Returns
                        The column text.
                """
            ...

        def GetValue(self, column: Union[str, int]) -> Optional[Union[str, int, float, datetime.datetime]]:
            """
                Returns the value in a given column of the current row as a valid Python type.
                    Args
                        ``column``: the name of the column as a string or the column position (starting at ``1``).
                        If a position greater than the number of columns is given, the last column is returned.
                    Returns
                        The targeted column value.
                        Binary types are returned as a ``int`` value indicating the length of the binary field.
                """
            ...

        def GoToCell(self, row: int = ..., column: Union[int, str] = ...) -> None:
            """
                Moves the cursor to the specified row and column.
                    Args
                        ``row``: the row number as a numeric value starting at ``1``.
                        If the requested row exceeds the number of existing rows, the cursor is moved to the last row.
                        If this argument is not specified, then the row is not changed.

                        ``column``: the name of the column as a string or the column position (starting at ``1``).
                        If the requested column exceeds the number of existing columns, the cursor is moved
                        to the last column. If this argument is not specified, then the column is not changed.
                """
            ...

        def RemoveMenu(self, menuheader: str) -> bool:
            """
                Removes a menu entry from the data view menubar by its name.
                    Args
                        ``menuheader``: the case-sensitive name of the menu to be removed.
                        The name must not include the tilde (``~``) character.
                    Returns
                        ``True`` when successful.
                    Note
                        This method can remove menus that belong to the standard user interface
                        as well as menus that were programmatically added with the CreateMenu method.
                        The removal of standard menus is not permanent, and they will reappear
                        after the window is closed and reopened.
                """
            ...

        def Toolbars(self, toolbarname: str = ...) -> Union[TOOLBAR, Tuple[str, ...]]:
            """
                Returns either a list of the available toolbar names in the actual datasheet or a ``Toolbar``
                service instance.
                    Args
                        ``toolbarname``:  the usual name of one of the available toolbars.
                    Returns
                        A zero-based array of toolbar names when the argument is absent,
                        or a new ``Toolbar`` object instance from the ``SF_Widgets`` class.
            """
            ...

# #####################################################################################################################
#                       SFDialogs CLASS    (alias of SFDialogs Basic library)                                   ###
# #####################################################################################################################
class SFDialogs:
    """
        Management of dialogs. They may be defined with the Basic IDE or built from scratch.
        """

    # #########################################################################
    # SF_Dialog CLASS
    # #########################################################################
    class SF_Dialog(SFServices):
        """
            The ``Dialog`` service contributes to the management of dialogs created with the Basic Dialog Editor
            or dialogs created on-the-fly. Each instance of the ``Dialog`` class represents a single dialog box
            displayed to the user.

            A dialog box can be displayed in modal or in non-modal modes.

                In modal mode,
                    the box is displayed and the execution of the macro process is suspended
                    until one of the ``OK`` or ``Cancel`` buttons is pressed. In the meantime, user actions executed
                    on the box can trigger specific actions.

                In non-modal mode,
                    the dialog box is "floating" on the user desktop and the execution
                    of the macro process continues normally. A non-modal dialog closes when it is terminated
                    with the Terminate() method or when the LibreOffice session ends.
                    The window close button is inactive in non-modal dialogs.

            A dialog box disappears from memory after its explicit termination.
            """

        OKBUTTON: Literal[1]
        """ ``OK`` button has been pressed.         """
        CANCELBUTTON: Literal[0]
        """ ``Cancel`` button has been pressed.     """

        Caption: str
        """ Specify the title of the dialog.        """
        Height: int
        """ Specify the height of the dialog.       """
        Modal: bool
        """ Specifies if the dialog box is currently in execution in modal mode.    """
        Name: str
        """ The name of the dialog.                 """
        Page: int
        """ A dialog may have several pages that can be traversed by the user step by step.
        The Page property of the Dialog object defines which page of the dialog is active.  """
        Visible: bool
        """ Specify if the dialog box is visible on the desktop.
        By default it is not visible until the ``Execute()`` method is run and visible afterwards.  """
        XDialogModel: UNO
        """ The UNO representation (``com.sun.star.awt.XControlModel``) of the dialog model.        """
        XDialogView: UNO
        """ The UNO representation (``com.sun.star.awt.XControl``) of the dialog view.             """
        Width: int
        """ Specify the width of the dialog.        """

        OnFocusGained: SCRIPT_URI
        """ Get/set the macro triggered by the ``When receiving focus`` event."""
        OnFocusLost: SCRIPT_URI
        """ Get/set the macro triggered by the ``When losing focus`` event."""
        OnKeyPressed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Key pressed`` event."""
        OnKeyReleased: SCRIPT_URI
        """ Get/set the macro triggered by the ``Key released`` event."""
        OnMouseDragged: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse moved while button pressed`` event."""
        OnMouseEntered: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse inside`` event."""
        OnMouseExited: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse outside`` event."""
        OnMouseMoved: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse moved`` event."""
        OnMousePressed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse button pressed`` event."""
        OnMouseReleased: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse button released`` event."""
        
        def Activate(self) -> bool:
            """
                Set the focus on the current dialog instance.
                Probably called from after an event occurrence or to focus on a non-modal dialog.
                    Returns
                        ``True`` if focusing is successful.
                """
            ...

        def Center(self, parent: Union[DIALOG, BASE, DOCUMENT, CALC, WRITER, FORMDOCUMENT, DATASHEET] = ...) -> bool:
            """
                Centres the current dialogue box instance in the middle of a parent window.
                Without arguments, the method centres the dialogue box in the middle of the current window.
                    Args
                        ``parent``: an optional object that can be either,
                            - a ScriptForge dialog object,
                            - a ScriptForge document (Calc, Base, ...) object.
                    Returns
                        ``True`` when successful.
                """
            ...
        def CloneControl(
            self, sourcename: str, controlname: str, left: int = ..., top: int = ...) -> Optional[DIALOGCONTROL]:
            """
                Duplicate an existing control of any type in the actual dialog.
                The duplicated control is left unchanged. The new control can be relocated.
                    Args
                        ``sourcename``: the name of the control to duplicate.

                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``left``:  the left coordinate of the new control expressed in "``Map AppFont``" units.

                        ``top``:  the top coordinate of the new control expressed in "``Map AppFont``" units.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def Controls(self, controlname: str = ...) -> Union[DIALOGCONTROL, Tuple[str, ...]]:
            """
                Returns the list of the controls contained in the dialog or a dialog control object based on its name.
                    Args
                        ``controlname``: a valid control name as a case-sensitive string.
                        If absent the list is returned.
                    Returns
                        The list of available control names as strings when ``controlname`` is absent.
                        Otherwise, if ``controlname`` exists, an instance of the ``SFDialogs.SF_DialogControl`` class.
                """
            ...

        def CreateButton(self,
                         controlname: str,
                         place: Union[UNO, Tuple[int, int, int, int]],
                         toggle: bool = ...,
                         push: Literal["", "OK", "CANCEL"] = ...,
                         ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``Button`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``toggle``: when ``True`` a toggle button is created. Default = ``False``.

                        ``push``: button type "OK", "CANCEL" or empty string (default).
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateCheckBox(self,
                           controlname: str,
                           place: Union[UNO, Tuple[int, int, int, int]],
                           multiline: bool = ...,
                           ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``ComboBox`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``multiline``: when ``True`` the caption may be displayed on more than one line.
                        Default = ``False``.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateComboBox(self,
                           controlname: str,
                           place: Union[UNO, Tuple[int, int, int, int]],
                           border: Literal["3D", "FLAT", "NONE"] = ...,
                           dropdown: bool = ...,
                           linecount: int = ...
                           ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``ComboBox`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``dropdown``: when ``True`` (default), a drop-down button is displayed.

                        ``linecount``: the maximum line count displayed in the drop-down (default = 5).
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateCurrencyField(self,
                                controlname: str,
                                place: Union[UNO, Tuple[int, int, int, int]],
                                border: Literal["3D", "FLAT", "NONE"] = ...,
                                spinbutton: bool = ...,
                                minvalue: Union[int, float] = ...,
                                maxvalue: Union[int, float] = ...,
                                increment: int = ...,
                                accuracy: int = ...
                                ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``CurrenyField`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``spinbutton``: when ``True`` a spin button is present. Default = ``False``.

                        ``minvalue``:  the smallest value that can be entered in the control. Default = -1000000.

                        ``maxvalue``:  the largest value that can be entered in the control. Default = +1000000.

                        ``increment``:  the step when the spin button is pressed. Default = 1.

                        ``accuracy``:  the decimal accuracy. Default = 2 decimal digits.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateDateField(self,
                            controlname: str,
                            place: Union[UNO, Tuple[int, int, int, int]],
                            border: Literal["3D", "FLAT", "NONE"] = ...,
                            dropdown: bool = ...,
                            mindate: datetime.datetime = ...,
                            maxdate: datetime.datetime = ...,
                            ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``DateField`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``dropdown``: when ``True`` a dropdown button is shown. Default = ``False``.

                        ``mindate``:  the smallest date that can be entered in the control. Default = 1900-01-01.

                        ``maxdate``:  the largest date that can be entered in the control. Default = 2200-12-31.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateFileControl(self,
                              controlname: str,
                              place: Union[UNO, Tuple[int, int, int, int]],
                              border: Literal["3D", "FLAT", "NONE"] = ...,
                              ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``FileControl`` in the actual dialog.
                    Args
                            ``controlname``:  the name of the new control. It must not exist yet.

                            ``place``: the size and position expressed in "``APPFONT units``". Either:
                                - a tuple (X, Y, Width, Height).
                                - a ``com.sun.star.awt.Rectangle`` structure.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateFixedLine(self,
                            controlname: str,
                            place: Union[UNO, Tuple[int, int, int, int]],
                            orientation: Literal["H", "Horizontal", "V", "Vertical"],
                            ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``FixedLine`` in the actual dialog.
                    Args
                            ``controlname``:  the name of the new control. It must not exist yet.

                            ``place``: the size and position expressed in "``APPFONT units``". Either:
                                - a tuple (X, Y, Width, Height).
                                - a ``com.sun.star.awt.Rectangle`` structure.

                            ``orientation``: for horizontal orientation use "H" or "Horizontal",
                            for vertical orientation use "V" or "Vertical".
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateFixedText(self,
                            controlname: str,
                            place: Union[UNO, Tuple[int, int, int, int]],
                            border: Literal["3D", "FLAT", "NONE"] = ...,
                            multiline: bool = ...,
                            align: Literal["LEFT", "CENTER", "RIGHT"] = ...,
                            verticalalign: Literal["TOP", "MIDDLE", "BOTTOM"] = ...,
                            ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``FixedText`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: border kind, "3D" or "FLAT" or "NONE". Default is ``NONE``.

                        ``multiline``: when ``True`` the caption may be displayed on more than one line.
                        Default ``False``.

                        ``align``: horizontal alignment, "LEFT" (default) or "CENTER" or "RIGHT".

                        ``verticalalign``: vertical alignment, "TOP" (default) or "MIDDLE" or "BOTTOM".
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateFormattedField(self,
                                 controlname: str,
                                 place: Union[UNO, Tuple[int, int, int, int]],
                                 border: Literal["3D", "FLAT", "NONE"] = ...,
                                 spinbutton: bool = ...,
                                 minvalue: Union[int, float] = ...,
                                 maxvalue: Union[int, float] = ...,
                                 ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``FormattedField`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``spinbutton``: when ``True`` a spin button is present. Default = ``False``.

                        ``minvalue``:  the smallest value that can be entered in the control. Default = -1000000.

                        ``maxvalue``:  the largest value that can be entered in the control. Default = +1000000.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateGroupBox(self,
                           controlname: str,
                           place: Union[UNO, Tuple[int, int, int, int]],
                           ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``GroupBox`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateHyperlink(self,
                            controlname: str,
                            place: Union[UNO, Tuple[int, int, int, int]],
                            border: Literal["3D", "FLAT", "NONE"] = ...,
                            multiline: bool = ...,
                            align: Literal["LEFT", "CENTER", "RIGHT"] = ...,
                            verticalalign: Literal["TOP", "MIDDLE", "BOTTOM"] = ...,
                            ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``Hyperlink`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: border kind, "3D" or "FLAT" or "NONE". Default is ``NONE``.

                        ``multiline``: when ``True`` the caption may be displayed on more than one line.
                        Default ``False``.

                        ``align``: horizontal alignment, "LEFT" (default) or "CENTER" or "RIGHT".

                        ``verticalalign``: vertical alignment, "TOP" (default) or "MIDDLE" or "BOTTOM".
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateImageControl(self,
                               controlname: str,
                               place: Union[UNO, Tuple[int, int, int, int]],
                               border: Literal["3D", "FLAT", "NONE"] = ...,
                               scale: Literal["FITTOSIZE", "KEEPRATIO", "NO"] = ...,
                               ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``ImageControl`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: border kind, "3D" or "FLAT" or "NONE". Default is ``NONE``.

                        ``scale``: one of next values, "FITTOSIZE" (default) or "KEEPRATIO" or "NO".
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateListBox(self,
                          controlname: str,
                          place: Union[UNO, Tuple[int, int, int, int]],
                          border: Literal["3D", "FLAT", "NONE"] = ...,
                          dropdown: bool = ...,
                          linecount: int = ...,
                          multiselect: bool = ...
                          ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``ListBox`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``dropdown``: When ``True`` (default), a drop-down button is displayed.

                        ``linecount``: the maximum line count displayed in the drop-down (default = 5).

                        ``multiselect``: when ``True``, more than 1 entry may be selected. Default = ``False``.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateNumericField(self,
                               controlname: str,
                               place: Union[UNO, Tuple[int, int, int, int]],
                               border: Literal["3D", "FLAT", "NONE"] = ...,
                               spinbutton: bool = ...,
                               minvalue: Union[int, float] = ...,
                               maxvalue: Union[int, float] = ...,
                               increment: int = ...,
                               accuracy: int = ...
                               ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``NumericField`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``spinbutton``: when ``True`` a spin button is present. Default = ``False``.

                        ``minvalue``:  the smallest value that can be entered in the control. Default = -1000000.

                        ``maxvalue``:  the largest value that can be entered in the control. Default = +1000000.

                        ``increment``:  the step when the spin button is pressed. Default = 1.

                        ``accuracy``:  the decimal accuracy. Default = 2 decimal digits.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreatePatternField(self,
                               controlname: str,
                               place: Union[UNO, Tuple[int, int, int, int]],
                               border: Literal["3D", "FLAT", "NONE"] = ...,
                               editmask: str = ...,
                               literalmask: str = ...,
                               ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``PatternField`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``editmask``: a character code that determines what the user may enter. More info on
                        https://wiki.documentfoundation.org/Documentation/DevGuide/Graphical_User_Interfaces#Pattern_Field.

                        ``literalmask``: contains the initial values that are displayed in the pattern field.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateProgressBar(self,
                              controlname: str,
                              place: Union[UNO, Tuple[int, int, int, int]],
                              border: Literal["3D", "FLAT", "NONE"] = ...,
                              minvalue: Union[int, float] = ...,
                              maxvalue: Union[int, float] = ...,
                              ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``ProgressBar`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``minvalue``:  the smallest value that can be entered in the control. Default = 0.

                        ``maxvalue``:  the largest value that can be entered in the control. Default = 100.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
        ...

        def CreateRadioButton(self,
                              controlname: str,
                              place: Union[UNO, Tuple[int, int, int, int]],
                              border: Literal["3D", "FLAT", "NONE"] = ...,
                              multiline: bool = ...,
                              ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``RadioButton`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``multiline``: when ``True`` the caption may be displayed on more than one line.
                        Default is ``False``.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateScrollBar(self,
                            controlname: str,
                            place: Union[UNO, Tuple[int, int, int, int]],
                            orientation: Literal["H", "Horizontal", "V", "Vertical"],
                            border: Literal["3D", "FLAT", "NONE"] = ...,
                            minvalue: Union[int, float] = ...,
                            maxvalue: Union[int, float] = ...,
                            ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``ScrollBar`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``orientation``: for horizontal orientation use "H" or "Horizontal",
                        for vertical orientation use "V" or "Vertical".

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``minvalue``:  the smallest value that can be entered in the control. Default = 0.

                        ``maxvalue``:  the largest value that can be entered in the control. Default = 100.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateTableControl(self,
                               controlname: str,
                               place: Union[UNO, Tuple[int, int, int, int]],
                               border: Literal["3D", "FLAT", "NONE"] = ...,
                               rowheaders: bool = ...,
                               columnheaders: bool = ...,
                               scrollbars: Literal["H", "Horizontal", "V", "Vertical", "B", "Both", "N", "None"] = ...,
                               gridlines: bool = ...,
                               ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``TableControl`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``rowheaders``: when ``True`` (default), the row headers are shown.

                        ``columnheaders``: when ``True`` (default), the column headers are shown.

                        ``scrollbars``: H[orizontal] or V[ertical] or B[oth] or N[one] (default).

                        ``gridlines``: when ``True`` horizontal and vertical lines are painted between the grid cells.
                        Default is ``False``.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateTextField(self,
                            controlname: str,
                            place: Union[UNO, Tuple[int, int, int, int]],
                            border: Literal["3D", "FLAT", "NONE"] = ...,
                            multiline: bool = ...,
                            maximumlength: int = ...,
                            passwordcharacter: str = ...
                            ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``TextField`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: border kind, "3D" or "FLAT" or "NONE". Default is ``NONE``.

                        ``multiline``: when ``True`` the caption may be displayed on more than one line.
                        Default ``False``.

                        ``maximumlength``: the maximum character count (default = 0 meaning unlimited).

                        ``passwordcharacter``: a single character specifying the echo for a password text field
                        (default = "").
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateTimeField(self,
                            controlname: str,
                            place: Union[UNO, Tuple[int, int, int, int]],
                            border: Literal["3D", "FLAT", "NONE"] = ...,
                            mintime: datetime.datetime = ...,
                            maxtime: datetime.datetime = ...
                            ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``TimeField`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".

                        ``mintome``:  the smallest time that can be entered in the control. Default = 0h.

                        ``maxtime``:  the largest time that can be entered in the control. Default = 24h.
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def CreateTreeControl(self,
                              controlname: str,
                              place: Union[UNO, Tuple[int, int, int, int]],
                              border: Literal["3D", "FLAT", "NONE"] = ...,
                              ) -> Optional[DIALOGCONTROL]:
            """
                Create a new control of type ``TreeControl`` in the actual dialog.
                    Args
                        ``controlname``:  the name of the new control. It must not exist yet.

                        ``place``: the size and position expressed in "``APPFONT units``". Either:
                            - a tuple (X, Y, Width, Height).
                            - a ``com.sun.star.awt.Rectangle`` structure.

                        ``border``: "3D" (default), "FLAT" or "NONE".
                    Returns
                        A ``SFDialogs.SF_DialogControl`` instance or ``None``.
                """
            ...

        def EndExecute(self, returnvalue: int) -> None:
            """
                Ends the display of a modal dialog and gives back the argument
                as return value for the current Execute() action.

                ``EndExecute()`` is usually contained in the processing of a macro
                triggered by a dialog or control event.
                    Args
                        ``returnvalue``: The value passed to the running ``Execute()`` method.
                """
            ...

        def Execute(self, modal: bool = ...) -> int:
            """
                Display the dialog. In modal mode, the process is suspended until its closure by the user.
                    Args
                        ``modal``: ``False`` when non-modal dialog. Defaults to ``True``.
                    Returns
                        0 = Cancel button pressed. 1 = OK button pressed. Otherwise: the dialog stopped
                        with an ``EndExecute()`` statement executed from a dialog or control event.
                """
            ...

        def GetTextsFromL10N(self, l10n: L10N) -> bool:
            """
                Replace all fixed text strings of a dialog by their localized version.
                Replaced texts are:
                    - the title of the dialog
                    - the caption associated with next control types: ``Button, CheckBox, FixedLine, FixedText, GroupBox`` and ``RadioButton``
                    - the content of list- and comboboxes
                    - the tip- or helptext displayed when the mouse is hovering a control.
                The current method has a twin method ``ScriptForge.SF_L10N.AddTextsFromDialog``.
                The current method is probably run before the ``Execute()`` method.
                    Args
                        ``l10n``: A "L10N" service instance created with CreateScriptService("L10N").
                    Returns
                        ``True`` when successful.
                """
            ...

        def OrderTabs(self, tabslist: Sequence[str], start: int = ..., increment: int = ...) -> bool:
            """
                Set the tabulation index of a series of controls.

                The sequence of controls is given as a list or tuple of control names from the first to the last.

                Next controls will not be accessible (anymore ?) via the TAB key if >=1 of next conditions is met:
                    - if they are not in the given list
                    - if their type is ``FixedLine``, ``GroupBox`` or ``ProgressBar``
                    - if the control is disabled
                Args
                    ``tabslist``: a list or tuple of valid control names in the order of tabulation.

                    ``start``: the tab index to be assigned to the 1st control in the list. Default is ``1``.

                    ``increment``: the difference between ``2`` successive tab indexes. Default is ``1``.
                Returns
                    ``True`` when successful.
                """
            ...

        def Resize(self, left: int = ..., top: int = ..., width: int = ..., height: int = ...) -> bool:
            """
                Moves the topleft corner of a dialog to new coordinates and/or modify its dimensions.
                All distances are expressed in ``Map AppFont`` units.
                Without arguments, the method resets the initial position and dimensions.
                    Args
                        ``left``: the horizontal distance from the top-left corner.

                        ``top``: the vertical distance from the top-left corner.

                        ``width``: the width of the rectangle containing the dialog.

                        ``height``: the height of the rectangle containing the dialogue box.
                    Returns
                        ``True`` if the resize was successful.
                """
            ...

        def SetPageManager(self,
                           pilotcontrols: str = ...,
                           tabcontrols: str = ...,
                           wizardcontrols: str = ...,
                           lastpage: int = ...,
                           ) -> bool:
            """
                Defines which controls in a dialog are responsible for switching pages, making it easier
                to orchestrate the ``Page`` property of a dialog and its controls.

                Dialogs may have multiple pages and the currently visible page is defined by the ``Page``dialog property.
                If the ``Page`` property is left unchanged, the default visible page is equal to ``0`` (zero), meaning
                that no particular page is defined and all visible controls are displayed regardless of the value set in
                their own ``Page`` property.

                When the ``Page`` property of a dialog is changed to some other value such as ``1``, ``2``, ``3``
                and so forth, then only the controls whose ``Page`` property match the current dialog page will
                be displayed.

                By using the SetPageManager method it is possible to define four types of page managers:
                    - List box or combo box: in this case, each entry in the list box or combo box corresponds to a page. The first item refers to Page 1, the second items refers to Page 2 and so on.
                    - Group of radio buttons: defines a group of radio buttons that will control which page is visible.
                    - Sequence of buttons: defines a set of buttons, each of which corresponding to a dialog page. This can be used to emulate a tabbed interface by placing buttons side by side in the dialog.
                    - Previous/Next buttons: defines which buttons in the dialog that will be used to navigate to the Previous/Next page in the dialog.

                This method is supposed to be called just once before calling the ``Execute()`` method.
                Subsequent calls are ignored.
                    Args
                        ``pilotcontrols``: a comma-separated list of ``ListBox``, ``ComboBox`` or ``RadioButton``
                        control names used as page managers.
                        For ``RadioButton`` controls, specify the name of the first control in the group to be used.

                        ``tabcontrols``: a comma-separated list of button names that will be used as page managers.
                        The order in which they are specified in this argument corresponds to the page number
                        they are associated with.

                        ``wizardcontrols``: a comma-separated list with the names of two buttons that will be used
                        s the ``Previous/Next`` buttons.

                        ``lastpage``: the number of the last available page.
                        It is recommended to specify this value when using the ``Previous/Next`` page manager.
                    Returns
                        ``True`` on success.
                    Tip
                        It is possible to use more than one page management mechanism at the same time.
                """
            ...

        def Terminate(self) -> bool:
            """
                Terminate the dialog service for the current dialog instance.
                After termination any action on the current instance will be ignored.
                    Returns
                        ``True`` if termination is successful.
                """
            ...

    # #########################################################################
    # SF_DialogControl CLASS
    # #########################################################################
    class SF_DialogControl(SFServices):
        """
            Each instance of the current class represents a single control within a dialog box.
            The focus is clearly set on getting and setting the values displayed by the controls of the dialog box,
            not on their formatting.
            A special attention is given to controls with type ``TreeControl``.
            """

        Border: Literal["3D", "FLAT", "NONE"]
        """ Get the surrounding of the control. """
        Cancel: bool
        """ Get/set whether a command button has or not the behaviour of a Cancel button.
        Applicable to ``Button`` controls.  """
        Caption: str
        """ Get/set the text associated with the control. Applicable to ``Button, CheckBox, FixedLine,
        FixedText, GroupBox, Hyperlink, RadioButton`` controls.  """
        ControlType: str
        """ get the type of control as a string.    """
        CurrentNode: UNO
        """ Get/set the currently upmost node selected in the tree control, as a
        ``com.sun.star.awt.tree.XMutableTreeNode`` object. Applicable to ``TreeControl`` controls. """
        Default: bool
        """ Get/set whether a command button is the default (OK) button.
        Applicable to ``Button`` controls.  """
        Enabled: bool
        """ Specifies if the control is accessible with the cursor. """
        Format: Literal["Standard (short)", "Standard (short YY)", "Standard (short YYYY)", "Standard (long)",
        "DD/MM/YY", "MM/DD/YY", "YY/MM/DD", "DD/MM/YYYY", "MM/DD/YYYY", "YYYY/MM/DD", "YY-MM-DD", "YYYY-MM-DD",
        "24h short", "24h long", "12h short", "12h long"]
        """ Get/set the format used to display dates and times. Applicable to ``DateField, TimeFiels, FormattedField``
        controls.   """
        Height: int
        """ Get/set the height of the control.       """
        ListCount: int
        """ Get the number of rows in the control. Applicable to ``ComboBox, ListBox, TableControl`` controls.  """
        ListIndex: int
        """ Get/set which item is selected  in the control. Applicable to ``ComboBox, ListBox, TableControl``
        controls.  """
        Locked: bool
        """ Get/set  if the control is read-only.. Applicable to ``ComboBox, CurrencyField, DateField, FileControl,
        FormattedField, ListBox, NumericField, PatternField, TextField, TimeField`` controls.   """
        MultiSelect: bool
        """ Get/set whether a user can make multiple selections in a ``listbox``.   """
        Name: str
        """ The name of the control.    """

        OnActionPerformed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Execute action`` event. """
        OnAdjustmentValueChanged: SCRIPT_URI
        """ Get/set the macro triggered by the ``While adjusting`` event. """
        OnFocusGained: SCRIPT_URI
        """ Get/set the macro triggered by the ``When receiving focus`` event."""
        OnFocusLost: SCRIPT_URI
        """ Get/set the macro triggered by the ``When losing focus`` event."""
        OnItemStatusChanged: SCRIPT_URI
        """ Get/set the macro triggered by the ``Item status changed`` event. """
        OnKeyPressed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Key pressed`` event."""
        OnKeyReleased: SCRIPT_URI
        """ Get/set the macro triggered by the ``Key released`` event."""
        OnMouseDragged: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse moved while button pressed`` event."""
        OnMouseEntered: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse inside`` event."""
        OnMouseExited: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse outside`` event."""
        OnMouseMoved: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse moved`` event."""
        OnMousePressed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse button pressed`` event."""
        OnMouseReleased: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse button released`` event."""
        OnNodeExpanded: SCRIPT_URI
        """ Get/set the macro triggered by the ``Expansion button is pressed on a node in a tree control`` event. """
        OnNodeSelected: SCRIPT_URI
        """ Get/set the macro triggered by the ``Node in a tree control is selected`` event."""

        Page: int
        """ A dialog may have several pages that can be traversed by the user step by step.
        The Page property of the Dialog object defines which page of the dialog is active.
        The`` ``Page property of a control defines the page of the dialog on which the control is visible. """
        Parent: DIALOG
        """ The parent ``DIALOG`` class object instance.    """
        Picture: FILE
        """ Get/se the file name (in ``FileSystem.FileNaming`` notation) containing a bitmap or other type of graphic
        to be displayed on the specified control.   """
        RootNode: UNO
        """ Get the lowest root node (usually there is only one such root node), as a
        ``com.sun.star.awt.tree.XMutableTreeNode`` object. Applicable to ``TreeControl`` controls. """
        RowSource: Sequence[str]
        """ Get/set the data contained in a ``combobox`` or a ``listbox``.  """
        TabIndex: int
        """ Specifies a control's place in the tab order in the dialog. """
        Text: str
        """ Get the text being displayed by the control. Applicable to ``ComboBox, FileControl, FormattedField,
        PatternField, TextField`` controls.  """
        TripleState: bool
        """ Get/set whether the ``checkbox`` control may appear dimmed (grayed).    """
        URL: str
        """ Get/set the URL to open when clicking the ``hyperlink`` control. """
        Value: Any
        """
        Get/set the content of the control:

        - ``Button``: bool - For toggle buttons only.
        - ``CheckBox``: bool, int - 0, ``False``: not checked, 1, ``True``: checked, 2: grayed, don't know.
        - ``ComboBox``: str - The selected value. The ``ListIndex`` property is an alternate option.
        - ``CurrencyField``: int, float.
        - ``DateField``: datetime.datetime.
        - ``FileControl``: FILE.
        - ``FormattedField``: str, int, float.
        - ``ListBox``: List(str), str - The selected row(s) as a scalar or as an array depending on the ``MultiSelect`` attribute.
        - ``NumericField``: int, float.
        - ``PatternField``: str.
        - ``ProgressBar``: int - Must be within the predefined bounds.
        - ``RadioButton``: bool - Each button has its own name. They are linked together if their TAB positions are contiguous. If a radiobutton is set to ``True``, the other related buttons are automatically set to ``False``.
        - ``ScrollBar``: int - Must be within the predefined bounds.
        - ``TableControl``: List[Any] - The data of the currently selected row.
        - ``TextField``: str - The text appearing in the control.
        - ``TimeField``: datetime.datetime.
        Not applicable to ``FixedLine, FixedText, GroupBox, Hyperlink, ImageControl`` and ``TreeControl`` dialog controls.
        """
        Visible: bool
        """ Get/set if the dialog control is hidden or visible.  """
        Width: int
        """ Get/set the width of the control.        """
        X: int
        """ X coordinate of the top-left corner of the control. """
        Y: int
        """ Y coordinate of the top-left corner of the control. """

        XControlModel: UNO
        """ The UNO representation (``com.sun.star.awt.XControlModel``) of the control model.        """
        XControlView: UNO
        """ The UNO representation (``com.sun.star.awt.XControl``) of the control view.  """
        XGridColumnModel: UNO
        """ The UNO representation (``com.sun.star.awt.grid.XGridColumnModel``) of a ``tablecontrol`` column model.  """
        XGridDataModel: UNO
        """ The UNO representation (``com.sun.star.awt.grid.XGridDataModel``) of a ``tablecontrol`` data model.  """
        XTreeDataModel: UNO
        """ The UNO representation (``com.sun.star.awt.tree.MutableTreeDataModel``) of a ``treecontrol`` data model.  """

        def AddSubNode(self,
                       parentnode: XMutableTreeNode,
                       displayvalue: str,
                       datavalue: Any = ...,
                       ) -> XMutableTreeNode:
            """
                Return a new node of the tree control subordinate to a parent node.
                    Args
                        ``parentnode``: a node UNO object, of type ``com.sun.star.awt.tree.XMutableTreeNode``.

                        ``displayvalue``: the text appearing in the control box.

                        ``datavalue``: any value associated with the new node.
                    Returns
                        The new node UNO object: ``com.sun.star.awt.tree.XMutableTreeNode``.
                """
            ...
        
        def AddSubTree(self,
                       parentnode: XMutableTreeNode,
                       flattree: MATRIX,
                       withdatavalue: bool = ...,
                       ):
            """
                Return ``True`` when a subtree, subordinate to a parent node, could be inserted successfully
                in a tree control. If the parent node had already child nodes before calling this method,
                the child nodes are erased.
                    Args
                        ``parentnode``: a node UNO object, of type ``com.sun.star.awt.tree.XMutableTreeNode``.
                        
                        ``flattree``: a list of lists sorted on the columns containing the ``DisplayValues``.
                        
                        ``withdatavalue``: when ``False`` (default), every column of ``FlatTree`` contains the
                        text to be displayed in the tree control. When ``True``, the texts to be displayed
                        (``DisplayValue``) are in columns 0, 2, 4, ... while the ``DataValues`` are in columns
                        1, 3, 5, ...
                    Notes
                        Typically, such an array can be issued by the GetRows() method applied on
                        the ``SFDatabases.Database`` service.
                """
            ...
        
        def CreateRoot(self, displayvalue: str, datavalue: Any = ...) -> XMutableTreeNode:
            """
                Return a new root node of the tree control.
                The new tree root is inserted below pre-existing root nodes.
                    Args
                        ``displayvalue``: the text appearing in the control box.

                        ``datavalue``: any value associated with the root node.
                    Returns
                        The new root node as a UNO object of type ``com.sun.star.awt.tree.XMutableTreeNode``.
                """
            ...

        def FindNode(self,
                     displayvalue: str = ...,
                     datavalue: Any = ...,
                     casesensitive: bool = ...,
                     ) -> Optional[XMutableTreeNode]:
            """
                Traverses the tree and find recursively, starting from the root, a node meeting some criteria.
                Either (1 match is enough) having its ``DisplayValue`` like ``displayValue`` or
                having its ``DataValue`` = ``datavalue``.

                Comparisons may be or not case-sensitive.

                The first matching occurrence is returned.
                    Args
                        ``displayvalue``: the pattern to be matched. It may contain wildcards(? and *).

                        ``datavalue``: a string, a numeric value or a date.

                        ``casesensitive``: Defaults to ``False``.
                    Returns
                        The found node of type ``com.sun.star.awt.tree.XMutableTreeNode`` or ``None`` if not found.
                """
            ...

        def Resize(self, left: int = ..., top: int = ..., width: int = ..., height: int = ...) -> bool:
            """
                Move the top-left corner of the control to new coordinates and/or modify its dimensions.
                Without arguments, the method resets the initial dimensions and position.
                All distances are expressed in ``Map AppFont`` units and are measured from the top-left corner
                of the parent dialog.
                    Args
                        ``left``: the horizontal distance from the top-left corner. It may be negative.

                        ``top``: the vertical distance from the top-left corner. It may be negative.

                        ``width``: the horizontal width of the rectangle containing the control. It must be positive.

                        ``height``: the vertical height of the rectangle containing the control. It must be positive.
                    Returns
                        ``True`` when successful.
                """
            ...

        def SetFocus(self) -> bool:
            """
                Set the focus on the current Control instance.
                Probably called from after an event occurrence.
                    Returns
                        ``True`` if focusing is successful.
                """
            ...

        def SetTableData(self,
                         dataarray: MATRIX,
                         widths: Sequence[Union[int, float]] = ...,
                         alignments: str = ...,
                         rowheaderwidth: int = ...,
                         ) -> bool:
            """
                Fill a table control with the given data. Preexisting data is erased.

                The Basic IDE allows to define if the control has a row and/or a column header.
                When it is the case, the array in argument should contain those headers resp. in the first
                column and/or in the first row.

                A column in the control shall be sortable when the data (headers excluded) in that column
                is homogeneously filled either with numbers or with strings.

                Columns containing strings will by default be left-aligned, those with numbers will be right-aligned.
                    Args
                        ``dataarray``: the set of data to display in the table control, including optional
                        column/row headers.

                        ``widths``: tuple or list containing the relative widths of each column.
                            In other words, widths = (1, 2) means that the second column is twice as wide as
                            the first one. If the number of values in the tuple is smaller than the number of
                            columns in the table, then the last value in the tuple is used to define the width
                            of the remaining columns.

                        ``alignments``: the column's horizontal alignment as a string with length = number of columns.
                        Possible characters are: L(eft), C(enter), R(ight) or space (default behavior).

                        ``rowheaderwidth``: width of the row header column expressed in ``Map AppFont`` units.
                        Defaults to ``10``. The argument is ignored when the table control has no row header.
                    Returns
                        ``True`` when successful.
                """
            ...

        def WriteLine(self, line: str = ...) -> bool:
            """
                Add a new line to a multiline ``TextField`` control.
                    Args
                        ``line``: the line to insert at the end of the text box.
                        A newline character will be inserted before the line, if relevant. Defaults to an empty line.
                    Returns
                        ``True`` if insertion is successful.
                """
            ...


# #####################################################################################################################
#                       SFDocuments CLASS    (alias of SFDocuments Basic library)                                   ###
# #####################################################################################################################
class SFDocuments:
    """
        The SFDocuments class gathers a number of classes, methods and properties making easy
        managing and manipulating LibreOffice documents.
        """

    # #########################################################################
    # SF_Document CLASS
    # #########################################################################
    class SF_Document(SFServices):
        """
            The methods and properties are generic for all types of documents: they are combined in the
            current SF_Document class
                - saving, exporting, closing documents
                - accessing their standard or custom properties
                - styles and menubar management

            Specific properties and methods are implemented in the concerned subclass(es) SF_Calc, SF_Base, ...
            """

        CustomProperties: DICTIONARY
        """ Returns a ``ScriptForge.Dictionary`` object instance. After update, can be passed again to the property
        for updating the document. Individual items of the dictionary may be either strings, numbers, 
        ``datetime.datetime``  or ``com.sun.star.util.Duration`` items.
        This property is not applicable to ``Base`` documents. """
        Description: str
        """ Get/set the ``Description`` property of the document (also known as "``Comments``").
        This property is not applicable to ``Base`` documents. """
        DocumentProperties: DICTIONARY
        """ Returns a ``ScriptForge.Dictionary`` object instance containing all the entries. Document statistics
        are included. Note that they are specific to the type of document. As an example, a ``Calc`` document
        includes a "``CellCount``" entry. Other documents do not.
        This property is not applicable to ``Base`` documents. """
        DocumentType: str
        """ String value with the document type (``Base``, ``Calc``, ``Writer``, etc) """
        ExportFilters: Tuple[str, ...]
        """ Returns a tuple of strings with the export filter names applicable to the current document.
        Filters used for both import/export are also returned.
        This property is not applicable to ``Base`` documents. """
        FileSystem: FILE
        """ Returns a string with the URL path to the root of the virtual file system of the document.
        Use the ``FileSystem`` service to view its contents, as well as to create, open and read files stored in it. """
        ImportFilters: Tuple[str, ...]
        """ Returns a tuple of strings with the import filter names applicable to the current document.
        Filters used for both import/export are also returned.
        This property is not applicable to ``Base`` documents. """
        IsBase: bool
        """ ``True`` when type of document = ``Base``.          """
        IsCalc: bool
        """ ``True`` when type of document = ``Calc``.          """
        IsDraw: bool
        """ ``True`` when type of document = ``Draw``.          """
        IsFormDocument: bool
        """ ``True`` when type of document = ``FormDocument``.  """
        IsImpress: bool
        """ ``True`` when type of document = ``Impress``.       """
        IsMath: bool
        """ ``True`` when type of document = ``Math``.          """
        IsWriter: bool
        """ ``True`` when type of document = ``Writer``.        """
        Keywords: str
        """ Get/set the ``Keywords`` property of the document as a comma-seprated list of keywords.
        This property is not applicable to ``Base`` documents. """
        Readonly: bool
        """ ``True`` if the document is actually in read-only mode.
        This property is not applicable to ``Base`` documents. """
        StyleFamilies: Tuple[str, ...]
        """ Get the list of available style families.
        This property is not applicable to ``Base`` documents. """
        Subject: str
        """ Get/set the ``Subject`` property of the document.
        This property is not applicable to ``Base`` documents. """
        Title: str
        """ Get/set the ``Title`` property of the document.
        This property is not applicable to ``Base`` documents. """

        XComponent: UNO
        """ A ``com.sun.star.lang.XComponent`` or ``com.sun.star.comp.dba.ODatabaseDocument`` UNO object representing
        the document. """
        XDocumentSettings: UNO
        """ A ``com.sun.star.XXX.DocumentSettings`` UNO object, where XXX is either ``sheet, text, drawing`` or
        ``presentation``. This object gives access to the internal UNO properties that are specific
        to the document's type.
        This property is not applicable to ``Base`` documents. """

        def Activate(self) -> bool:
            """
                Make the current document active.
                    Returns
                        ``True`` if the document could be activated.
                        Otherwise, there is no change in the actual user interface.
                """
            ...

        def CloseDocument(self, saveask: bool = ...) -> bool:
            """
                Close the document. Does nothing if the document is already closed
                regardless of how the document was closed, manually or by program.
                    Args
                        ``saveask``: if ``True`` (default), the user is invited to confirm or not the writing
                        of the changes on disk. No effect if the document was not modified.
                    Returns
                        ``False`` if the user declined to close.
                """
            ...

        def CreateMenu(self, menuheader: str, before: Union[str, int] = ..., submenuchar: str = ...
                       ) -> MENU:
            """
                Creates a new menu entry in the menubar of a given document window.

                The menu created is only available during the current LibreOffice session and is not saved neither
                in the document nor in the global application settings.
                Hence, closing the document window will make the menu disappear.
                It will only reappear when the macro that creates the menu is executed again.
                    Args
                        ``menuheader``: the toplevel name of the new menu.

                        ``before``: the name (as a string) or position (as an integer starting at 1) of
                        an existing menu before which the new menu will be placed.
                        If no value is defined for this argument, the menu will be created at the last position
                        in the menubar.

                        ``submenuchar``: the delimiter used to create menu trees when calling
                        methods as ``AddItem()`` from the Menu service. The default value is ">".
                    Returns
                        A ``SFWidgets.SF_Menu`` class instance or ``None``.
                """
            ...

        def DeleteStyles(self, family: str, styleslist: Union[str, Sequence[str]]) -> None:
            """
                Suppresses a single style or a list of styles given by their names within a specific styles family.
                Only user-defined styles may be deleted, built-in styles are ignored.
                It applies to all document types except ``Base`` and ``FormDocument``.
                    Args
                        ``family``: one of the style families present in the actual document, as a case-sensitive
                        string. Valid family names can be retrieved using the ``StyleFamilies`` property.

                        ``styleslist``: a single style name as a string or a list/tuple of style names.
                        The style names may be localized or not. The styles list is typically the output of the
                        execution of a ``Styles()`` method.

                """
            ...

        def Echo(self, echoon: bool = ..., hourglass: bool = ...) -> None:
            """
                While a script is executed any display update resulting from that execution
                is done immediately.

                For performance reasons it might be an advantage to differ the display updates
                up to the end of the script. This is where pairs of Echo() methods to set and reset the removal of the
                immediate updates may be beneficial.

                Optionally the actual mouse pointer can be modified to the image of an hourglass.

                    Args
                        ``echoon``: when ``False``, the display updates are suspended. Default is ``True``.

                        ``hourglass``: when ``True``, the mouse pointer is changed to an hourglass. Default is ``False``.
                """
            ...

        def ExportAsPDF(self,
                        filename: FILE,
                        overwrite: bool = ...,
                        pages: str = ...,
                        password: str = ...,
                        watermark: str = ...,
                        ) -> bool:
            """
                Store the document to the given file location in PDF format.
                This method is not applicable to ``Base`` documents.
                    Args
                        ``filename``: identifies the file where to save.
                        It must follow the ``SF_FileSystem.FileNaming`` notation.

                        ``overwrite``: ``True`` if the destination file may be overwritten. Defaults to ``False``.

                        ``pages``: the pages to print as a string, like in the user interface. Example: "1-4;10;15-18".

                        ``password``: password to open the document.

                        ``watermark``: the text for a watermark to be drawn on every page of the exported PDF file.
                    Returns
                            ``False`` if the document could not be saved.
                """
            ...

        def PrintOut(self, pages: str = ..., copies: int = ...) -> bool:
            """
                Send the content of the document to the printer.
                The printer might be defined previously by default, by the user or by the ``SetPrinter()`` method.
                This method is not applicable to ``Base`` documents.
                    Args
                        ``pages``: the pages to print as a string, like in the user interface. Example: "1-4;10;15-18".
                        Default is all pages.

                        ``copies``: the number of copies. Defaults to 1.
                    Returns
                        ``True`` when successful.
                """
            ...

        def RemoveMenu(self, menuheader: str) -> bool:
            """
                Removes a toplevel menu from the menubar of a given document window.
                    Args
                        ``menuheader``: the toplevel name of the menu to be removed.
                    Returns
                        ``True`` if the specified menu existed and could be removed.
                    Notes
                        This method can be used to remove any menu entry from the document window, including
                        default menus. However, none of these changes in the menubar are saved to the document
                        or to the application settings. To restore the menubar to the default settings,
                        simply close and reopen the document.
                """
            ...

        def RunCommand(self, command: str, *args, **kwargs) -> None:
            """
                Runs a UNO command on the document window associated with the service instance.

                The document itself does not need to be active to be able to run commands.

                Commands can be run with or without arguments. Arguments are not validated before running the command.
                If the command or its arguments are invalid, then nothing will happen.
                    Args
                        ``command``: case-sensitive string containing the UNO command name.
                        The inclusion of the prefix ``.uno:`` in the command is optional.
                        The command itself is not checked for correctness.

                        ``kwargs``: the command arguments as keyword arguments.
                """
            ...

        def Save(self) -> bool:
            """
                Store the document to the file location from which it was loaded.
                The method is ignored if the document was not modified.
                    Returns
                        ``False`` if the document could not be saved.
                """
            ...

        def SaveAs(self,
                   filename: FILE,
                   overwrite: bool = ...,
                   password: str = ...,
                   filtername: str = ...,
                   filteroptions: str = ...,
                   ) -> bool:
            """
                Store the document to the given file location.
                The new location becomes the new file name on which simple ``Save()`` method calls will be applied.
                    Args
                        ``filename``: identifies the file where to save. It must follow the ``SF_FileSystem.FileNaming``
                        notation.

                        ``overwrite``: ``True`` if the destination file may be overwritten. Defaults to ``False``.

                        ``password``: a non-space string to protect the document.

                        ``filtername``: the name of a filter that should be used for saving the document.
                        If present, the filter must exist.

                        ``filteroptions``: a string of options associated with the filter.
                    Returns
                        ``False`` if the document could not be saved.
                """
            ...

        def SaveCopyAs(self,
                       filename: FILE,
                       overwrite: bool = ...,
                       password: str = ...,
                       filtername: str = ...,
                       filteroptions: str = ...,
                       ) -> bool:
            """
                Store the document to the given file location.
                The actual location is unchanged
                    Args
                        ``filename``: identifies the file where to save. It must follow the ``SF_FileSystem.FileNaming``
                        notation.

                        ``overwrite``: ``True`` if the destination file may be overwritten. Defaults to ``False``.

                        ``password``: a non-space string to protect the document.

                        ``filtername``: the name of a filter that should be used for saving the document.
                        If present, the filter must exist.

                        ``filteroptions``: a string of options associated with the filter.
                    Returns
                        ``False`` if the document could not be saved.
                """
            ...

        def SetPrinter(self,
                       printer: str = ...,
                       orientation: Literal['PORTRAIT', 'LANDSCAPE'] = ...,
                       paperformat: Literal['A3', 'A4', 'A5', 'LETTER', 'LEGAL', 'TABLOID'] = ...
                       ) -> bool:
            """
                Define the printer options for the document.
                This method is not applicable to ``Base`` documents.
                    Args
                        ``printer``: the name of the printer queue where to print to.
                        When absent or space, the default printer is set.

                        ``orientation``: either ``PORTRAIT`` or ``LANDSCAPE``. Left unchanged when absent.

                        ``paperformat``: paper format. Left unchanged when absent.
                    Returns
                        ``True`` when successful.
                """
            ...

        def Styles(self,
                   family: str,
                   namepattern: str = ...,
                   parentstyle: str = ...,
                   used: bool = ...,
                   userdefined: bool = ...,
                   category: Literal['TEXT', 'CHAPTER', 'LIST', 'INDEX', 'EXTRA', 'HTML'] = ...,
                   ) -> Tuple[str, ...]:
            """
                Retrieves a list of styles matching an optional compound criteria, the returned list may be empty.
                This method is not applicable to ``Base`` documents.
                    Args
                        ``family``: one of the style families present in the actual document, as a case-sensitive
                        string. Valid family names can be retrieved using the ``StyleFamilies`` property.

                        ``namepattern``: a filter on the style names, as a case-sensitive string pattern.
                        The names include the internal and localized names. Admitted wildcards are "?" and "*".

                        ``parentstyle``: when present, only the children of the given, localized or not, parent style
                        name are retained.

                        ``used``: when ``True``, the style must be used in the document, when absent the argument
                        is ignored.

                        ``userdefined``: when ``True``, the style must have been added by the user, either in the
                        document or its template, when absent, the argument is ignored.

                        ``category``: a sub-category of the ``ParagraphStyles`` family.
                    Returns
                        A list of style names.
                """
            ...

        def Toolbars(self, toolbarname: str = ...) -> Union[TOOLBAR, Tuple[str, ...]]:
            """
                Returns either a list of the available toolbar names in the actual document
                or a ``SFWidgets.SF_Toolbar`` object instance.
                    Args
                        ``toolbarname``: the optional usual name of one of the available toolbars.
                    Returns
                        A list of toolbar names when the argument is absent, or a new ``Toolbar`` object instance
                        from the ``SF_Widgets`` library.
                """
            ...

        def XStyle(self, family: str, stylename: str) -> UNO:
            """
                This method returns the UNO representation of a given style.
                It is applicable to all document types except ``Base`` documents.
                    Args
                        ``family``: one of the style families present in the actual document, as a case-sensitive
                        string. Valid family names can be retrieved using ``StyleFamilies`` property.

                        ``stylename``: one of the styles present in the given family, as a case-sensitive string.
                        The ``stylename`` argument may be localized or not.
                    Returns
                        A ``com.sun.star.style.XStyle`` UNO object or one of its descendants,
                        like ``com.sun.star.style.CellStyle`` or ``com.sun.star.style.ParagraphStyle``, etc.
                        ``None`` is returned when the ``stylename`` does not exist in the given ``family``.
                """
            ...

    # #########################################################################
    # SF_Base CLASS
    # #########################################################################
    class SF_Base(SF_Document, SFServices):
        """
            The ``Base`` service provides a number of methods and properties to facilitate the management
            and handling of ``LibreOffice Base`` documents.

            The ``Base`` service extends the ``Document`` service and provides additional methods that are
            specific for ``Base`` documents, enabling users to:
                - get access to the database contained in a ``Base`` document
                - open form documents, tables and queries stored in a ``Base`` document
                - check if a form document from a Base document is currently loaded

            """

        def CloseFormDocument(self, formdocument: str) -> bool:
            """
                Close the given form document.
                Nothing happens if the form document is not open.

                The method is deprecated. Use the ``SF_FormDocument.CloseDocument()`` method instead.

                    Args
                        ``formdocument``: a valid hierarchical form name as a case-sensitive string.
                    Returns
                        ``True`` if closure is successful.
                """
            ...

        def FormDocuments(self) -> Tuple[str, ...]:
            """
                Return the list of the form documents contained in the ``Base`` document.
                    Returns
                        A tuple of strings. Each entry is the full path name of a form document.
                        The path separator is the slash ("/").
                """
            ...

        def Forms(self, formdocument: str, form: Union[str, int] = ...) -> Union[FORM, Tuple[str, ...]]:
            """
                Depending on the parameters provided this method will return:
                    - a tuple with the names of all the forms contained in a form document (if the ``form`` argument is absent)
                    - a ``SFDocuments.Form`` object representing the form specified in the ``form`` argument.

                The method is deprecated. Use the ``SF_FormDocument.Forms()`` method instead.
                    Args
                        ``formdocument``: the hierarchical name of an open form document.

                        ``form``: the name  or index number of the form stored in the form document.
                        If this argument is absent, the method will return a list with the names of all forms
                        available in the form document.
                    Returns
                        Either a tuple of strings. Each entry is a form name stored in the form document,
                        or a ``SFDocuments.Form`` class instance.
                """
            ...

        def GetDatabase(self, user: str = ..., password: str = ...) -> Optional[DATABASE]:
            """
                Returns a ``SFDatabases.Database`` class instance giving access
                to the execution of SQL commands on the database defined and/or stored in
                the actual ``Base`` document.
                    Args
                        ``user``, ``password``: the Login parameters as strings.
                        The default value for both parameters is the empty string.
                    Returns
                        A ``SFDatabases.SF_Database`` class instance or ``None``
                """
            ...

        def IsLoaded(self, formdocument: str) -> bool:
            """
                Return ``True`` if the given form Ddcument is currently open.
                    Args
                        ``formdocument``: A valid hierarchical form document name as a case-sensitive string.
                """
            ...

        def OpenFormDocument(self, formdocument: str, designmode: bool = ...) -> FORMDOCUMENT:
            """
                Open the form document given by its hierarchical name either in normal or in design mode.
                If the form document is already open, the form document is made active without changing its mode.
                    Args
                        ``formdocument``: a valid hierarchical form document name as a case-sensitive string.

                        ``designmode``: when ``True`` the form document is opened in design mode. Defaults to ``False``.
                    Returns
                        The ``SFDocuments.SF_FormDocument`` class instance corresponding with the opened form document.
                """
            ...

        def OpenQuery(self, queryname: str, designmode: bool = ...) -> DATASHEET:
            """
                Opens the Data View window of the specified query and returns an instance of the
                ``SFDatabases.SF_Datasheet`` service.

                The query can be opened in normal or design mode.

                If the query is already open, its Data View window will be made active.
                    Args
                        ``queryname``: the name of an existing SELECT query as a case-sensitive String.

                        ``designmode``: when ``True``, the query is opened in design mode.
                        Otherwise, it is opened in normal mode (default).
                    Returns
                        the ``SFDatabases.SF_Datasheet`` service instance corresponding with the query.
                    Note
                        Closing the Base document will cause the Data View window to be closed as well.
                """
            ...

        def OpenTable(self, tablename: str, designmode: bool = ...) -> DATASHEET:
            """
                Opens the Data View window of the specified table and returns an instance of the
                ``SFDatabases.SF_Datasheet`` service.

                The table can be opened in normal or design mode.

                If the table is already open, its Data View window will be made active.
                    Args
                        ``tablename``: the name of an existing table or view as a case-sensitive String.

                        ``designmode``: when ``True``, the table is opened in design mode.
                        Otherwise, it is opened in normal mode (default).
                    Returns
                        the ``SFDatabases.SF_Datasheet`` service instance corresponding with the table.
                    Note
                        Closing the Base document will cause the Data View window to be closed as well.
                """
            ...

        # pylint: disable=arguments-renamed
        def PrintOut(self, formdocument: str, pages: str = ..., copies: int = ...) -> bool:
            """
                Send the content of the form document to the printer.
                The printer might be defined previously by default, by the user or by the ``SetPrinter()`` method.

                The method is deprecated. Use the ``SF_FormDocument.PrintOut()`` method instead.

                    Args
                        ``formdocument``: a valid form document name as a case-sensitive string.
                        The form document must be open. It is activated by the method.

                        ``pages``: the pages to print as a string, like in the user interface. Example: "1-4;10;15-18".
                        Default is all pages.

                        ``copies``: the number of copies. Defaults to 1.
                    Returns
                        ``True`` when successful.
                """
            ...

        def SetPrinter(self,
                       formdocument: str = ...,
                       printer: str = ...,
                       orientation: Literal['PORTRAIT', 'LANDSCAPE'] = ...,
                       paperformat: Literal['A3', 'A4', 'A5', 'LETTER', 'LEGAL', 'TABLOID'] = ...
                       ) -> bool:
            """
                Define the printer options for the form document.

                The method is deprecated. Use the ``SF_FormDocument.SetPrinter()`` method instead.

                    Args
                        ``formdocument``: a valid form document name as a case-sensitive string.

                        ``printer``: the name of the printer queue where to print to.
                        When absent or space, the default printer is set.

                        ``orientation``: either ``PORTRAIT`` or ``LANDSCAPE``. Left unchanged when absent.

                        ``paperformat``: paper format. Left unchanged when absent.
                    Returns
                        ``True`` when successful.
                """
            ...

    # #########################################################################
    # SF_Calc CLASS
    # #########################################################################
    class SF_Calc(SF_Document, SFServices):
        """
            The ``SF_Calc`` class is focused on :
                - management (copy, insert, move, ...) of sheets within a ``Calc`` document
                - exchange of data between Python data structures and ``Calc`` ranges of values
                - copying and importing massive amounts of data

            All methods and properties defined for the ``Document`` service can also be accessed using a ``Calc``
            service instance.
            """

        CurrentSelection: Union[RANGE, Tuple[RANGE, ...]]
        """ Get/set the single selected range as a string or the list of selected ranges as a tuple of strings. """

        def FirstCell(self, rangename: RANGE) -> RANGE:
            """
                Returns the First used cell in a given range or sheet. When the argument is a sheet it will always
                return the "sheet.$A$1" cell. """
            ...
        def FirstColumn(self, rangename: RANGE) -> int:
            """ Returns the leftmost column number in a given range. """
            ...
        def FirstRow(self, rangename: RANGE) -> int:
            """ Returns the First used column in a given range. """
            ...
        def Height(self, rangename: RANGE) -> int:
            """ Returns the height in # of rows of the given range. """
            ...
        def LastCell(self, rangename: RANGE) -> RANGE:
            """ Returns the last used cell in a given sheet or range. """
            ...
        def LastColumn(self, rangename: RANGE) -> int:
            """ Returns the last used column in a given range or sheet. """
            ...
        def LastRow(self, rangename: RANGE) -> int:
            """ The last used row in a given range or sheet. """
            ...
        def Range(self, rangename: RANGE) -> CALCREFERENCE:
            """ Returns a (internal) range object """
            ...
        def Region(self, rangename: RANGE) -> RANGE:
            """
                Returns the address of the smallest area that contains the specified range
                so that the area is surrounded by empty cells or sheet edges.
                This is equivalent to applying the ``Ctrl + *`` shortcut to the given range.
                """
            ...
        def Sheet(self, sheetname: SHEETNAME) -> CALCREFERENCE:
            """
            Returns a sheet reference that can be used as argument of methods like ``CopySheet()``. """
            ...
        def SheetName(self, rangename: RANGE) -> str:
            """ Returns the sheet name part of a range address. """
            ...
        def Width(self, rangename: RANGE) -> int:
            """ The number of columns (>= 1) in the given range. """
            ...
        def XCellRange(self, rangename: RANGE) -> XCellRange:
            """ A ``com.sun.star.Table.XCellRange`` UNO object. """
            ...
        def XSheetCellCursor(self, rangename: str) -> XSheetCellCursor:
            """ A ``com.sun.star.sheet.XSheetCellCursor`` UNO object. """
            ...
        def XSpreadsheet(self, sheetname: SHEETNAME) -> XSpreadsheet:
            """ A ``com.sun.star.sheet.XSpreadsheet`` UNO object. """
            ...

        def A1Style(self, row1: int, column1: int, row2: int = ..., column2: int = ..., sheetname: SHEETNAME = ...,
                    ) -> RANGE:
            """
                Returns a range address as a string based on sheet coordinates, i.e. row and column numbers.

                If only a pair of coordinates is given, then an address to a single cell is returned.
                Additional arguments can specify the bottom-right cell of a rectangular range.

                Row and column numbers start at 1.

                    Args
                        ``row1``: specify the row number of the top cell in the range to be considered.

                        ``column1``: specify the column number of the left cell in the range to be considered.

                        ``row2``: specify the row number of the bottom cell in the range to be considered.
                        If this argument is not provided, or if ``row2 < row1``,
                        then the address of the single cell range represented by ``row1`` and ``column1`` is returned.

                        ``column2``: specify the column number of the right cell in the range to be considered.
                        If these arguments are not provided, or if ``column2 < rcolumn1``,
                        then the address of the single cell range represented by ``row1`` and ``column1`` is returned.

                        ``sheetname``: the name of the sheet to be prepended to the returned range address.
                        The sheet must exist. The default value is ``"~"`` corresponding to the currently active sheet.
                    Returns
                        A range address as a string.
                """
            ...

        def Activate(self, sheetname: SHEETNAME = ...) -> bool:
            """
                If the argument ``sheetname`` is provided, the given sheet is activated and becomes the currently
                selected sheet. If the argument is absent, then the document window is activated.
                    Args
                        ``sheetname``: the name of the sheet to be activated in the document.
                    Returns
                        ``True`` if the document or the sheet could be made active. Otherwise, there is no change
                        in the actual user interface.
                """
            ...

        def Charts(self, sheetname: SHEETNAME, chartname: str = ...) -> Union[Tuple[str, ...], CHART]:
            """
                Returns either the list with the names of all chart objects in a given sheet
                or a single Chart service instance.
                    - If only ``sheetname`` is specified, a tuple containing the names of all charts is returned.
                    - If a ``chartname`` is provided, then a single object corresponding to the desired chart is returned. The specified chart must exist.

                    Args
                        ``sheetname``: the name of the sheet from which the list of charts
                        is to be retrieved or where the specified chart is located.

                        ``chartname``: the user-defined name of the chart object to be returned.
                        If the chart does not have a user-defined name, then the internal object name can be used.
                        If this argument is absent, then the list of chart names in the specified sheet is returned.
                    Returns
                        Either the list with the names of all chart objects in a given sheet or a
                        single ``Chart`` service instance.
                """
            ...

        def ClearAll(self,
                     range: RANGE,
                     filterformula: str = ...,
                     filterscope: Literal['CELL', 'ROW', 'COLUMN'] = ...,
                     ) -> None:
            """
                Clears all the contents and formats of the given range.
                A filter formula can be specified to determine which cells shall be affected.
                    Args
                        ``range``: the range to be cleared, as a string.

                        ``filterformula``: a ``Calc`` formula that shall be applied to the given range
                        to determine which cells will be affected. The specified formula must return ``True``
                        or ``False``. If this argument is not specified, then all cells in the range are affected.
                            Express the formula in terms of (examples assume a range = ``"A1:J10"``):
                                - the top-left cell of the range when ``filterscope`` = "CELL" e.g. ``"=(A1>100)"``
                                - the topmost row of the range when ``filterscope`` = "ROW" e.g. ``"=(SUM($A1:$J1)<1000)"``
                                - the leftmost column of the range when ``filterscope`` = "COLUMN" e.g. ``"=(A$10=SUM(A$1:A$9))"``

                        ``filterscope``: determines how ``filterformula`` is expanded to the given range.
                        The argument is mandatory if a ``filterformula`` is specified.
                """
            ...

        def ClearFormats(self,
                         range: RANGE,
                         filterformula: str = ...,
                         filterscope: Literal['CELL', 'ROW', 'COLUMN'] = ...,
                         ) -> None:
            """
                Clears the formats of the given range.
                A filter formula can be specified to determine which cells shall be affected.
                    Args
                        ``range``: the range to be cleared, as a string.

                        ``filterformula``: a ``Calc`` formula that shall be applied to the given range
                        to determine which cells will be affected. The specified formula must return ``True``
                        or ``False``. If this argument is not specified, then all cells in the range are affected.
                            Express the formula in terms of (examples assume a range = ``"A1:J10"``):
                                - the top-left cell of the range when ``filterscope`` = "CELL" e.g. ``"=(A1>100)"``
                                - the topmost row of the range when ``filterscope`` = "ROW" e.g. ``"=(SUM($A1:$J1)<1000)"``
                                - the leftmost column of the range when ``filterscope`` = "COLUMN" e.g. ``"=(A$10=SUM(A$1:A$9))"``

                        ``filterscope``: determines how ``filterformula`` is expanded to the given range.
                        The argument is mandatory if a ``filterformula`` is specified.
                """
            ...

        def ClearValues(self,
                        range: RANGE,
                        filterformula: str = ...,
                        filterscope: Literal['CELL', 'ROW', 'COLUMN'] = ...,
                        ) -> None:
            """
                Clears the values and formulas in the given range.
                A filter formula can be specified to determine which cells shall be affected.
                    Args
                        ``range``: the range to be cleared, as a string.

                        ``filterformula``: a ``Calc`` formula that shall be applied to the given range
                        to determine which cells will be affected. The specified formula must return ``True``
                        or ``False``. If this argument is not specified, then all cells in the range are affected.
                            Express the formula in terms of (examples assume a range = ``"A1:J10"``):
                                - the top-left cell of the range when ``filterscope`` = "CELL" e.g. ``"=(A1>100)"``
                                - the topmost row of the range when ``filterscope`` = "ROW" e.g. ``"=(SUM($A1:$J1)<1000)"``
                                - the leftmost column of the range when ``filterscope`` = "COLUMN" e.g. ``"=(A$10=SUM(A$1:A$9))"``

                        ``filterscope``: determines how ``filterformula`` is expanded to the given range.
                        The argument is mandatory if a ``filterformula`` is specified.
                """
            ...
        def CompactLeft(self, range: RANGE, wholecolumn: bool = ..., filterformula: str = ...) -> RANGE:
            """
                Deletes the columns of a specified range that match a filter expressed as a ``Calc`` formula.
                The filter is applied to each column to decide whether it will be deleted or not.

                The deleted column can be limited to the height of the specified range or span to the
                height of the entire sheet, thus deleting whole columns.
                    Args
                        ``range``: the range from which columns will be deleted, as a string.

                        ``wholecolumn``: if this option is set to ``True`` the entire column will be deleted
                        from the sheet. The default value is ``False``, which means that the deleted column
                        will be limited to the height of the specified range.

                        ``filterformula``: the filter to be applied to each column to determine whether it will
                        be deleted. The filter is expressed as a ``Calc`` formula that should be applied
                        to the first column. When the formula returns ``True`` for a column, that column will be
                        deleted. The default filter deletes all empty columns.
                        For example, suppose range ``"A1:J200"`` is selected (height = 200), so the default formula
                        is ``"=(COUNTBLANK(A1:A200)=200)"``. This means that if all 200 cells are empty in the first
                        column (Column A), then the column is deleted.
                    Returns
                        String with the range address of the compacted range.
                        If all columns are deleted, then an empty string is returned.
                """
            ...

        def CompactUp(self,
                      range: RANGE,
                      wholerow: bool = ...,
                      filterformula: str = ...
                      ) -> RANGE:
            """
                Deletes the rows of a specified range that match a filter expressed as a ``Calc`` formula.
                The filter is applied to each row to decide whether it will be deleted or not.

                The deleted row can be limited to the width of the specified range or span to the
                width of the entire sheet, thus deleting whole rows.
                    Args
                        ``range``: the range from which rows will be deleted, as a string.

                        ``wholerow``: if this option is set to ``True`` the entire row will be deleted
                        from the sheet. The default value is ``False``, which means that the deleted row
                        will be limited to the width of the specified range.

                        ``filterformula``: the filter to be applied to each row to determine whether it will
                        be deleted. The filter is expressed as a ``Calc`` formula that should be applied
                        to the first row. When the formula returns ``True`` for a row, that row will be
                        deleted. The default filter deletes all empty rows.
                        For example, suppose range ``"A1:J200"`` is selected (width = 10), so the default formula
                        is ``"=(COUNTBLANK(A1:J1)=10)"``. This means that if all 10 cells are empty in the first
                        row (Row 1), then the row is deleted.
                    Returns
                        String with the range address of the compacted range.
                        If all rows are deleted, then an empty string is returned.
                """
            ...

        def CopySheet(self,
                      sheetname: Union[SHEETNAME, CALCREFERENCE],
                      newname: SHEETNAME,
                      beforesheet: Union[int, SHEETNAME]) -> bool:
            """
                Copies a specified sheet before an existing sheet or at the end of the list of sheets.
                The sheet to be copied may be contained inside any open ``Calc`` document.
                    Args
                        ``sheetname``: the name of the sheet to be copied as a string,
                        or a reference to a sheet as a ``CALCREFERENCE`` class instance.
                        A ``CALCREFERENCE`` is produced by the ``Sheet`` property of the ``Calc`` class.

                        ``newname``: the name of the sheet to insert. The name must not be in use in the document.

                        ``beforesheet``: the name (string) or index (numeric, starting from 1) of the sheet
                        before which to insert the copied sheet. The default behavior is to add the copied sheet
                        at the last position.
                    Returns
                        ``True`` if the sheet could be copied successfully.
                """
            ...

        def CopySheetFromFile(self,
                              filename: FILE,
                              sheetname: SHEETNAME,
                              newname: SHEETNAME,
                              beforesheet: Union[int, SHEETNAME] = ...,
                              ) -> bool:
            """
                Copies a specified sheet from a closed Calc document and pastes it before an existing
                sheet or at the end of the list of sheets.

                If the file does not exist, an error is raised. If the file is not a valid ``Calc`` file,
                a blank sheet is inserted. If the source sheet does not exist in the input file,
                an error message is inserted at the top of the newly pasted sheet.
                    Args
                        ``filename``: identifies the source file. It must follow the ``SF_FileSystem.FileNaming``
                        notation. The file must not be protected with a password.

                        ``sheetname``: the name of the sheet to be copied as a string.

                        ``newname``: the name of the copied sheet to be inserted in the document.
                        The name must not be in use in the document.

                        ``beforesheet``: the name (string) or index (numeric, starting from 1)
                        of the sheet before which to insert the copied sheet.
                        The default behavior is to add the copied sheet at the last position.
                    Returns
                        ``True`` if the sheet could be created.
                """
            ...

        def CopyToCell(self,
                       sourcerange: Union[RANGE, CALCREFERENCE],
                       destinationcell: RANGE
                       ) -> RANGE:
            """
                Copies a specified source range (values, formulas and formats) to a destination range or cell.
                The method reproduces the behavior of a Copy/Paste operation from a range to a single cell.

                The source range may belong to another open ``Calc`` document.

                    Args
                        ``sourcerange``: the source range as a string when it belongs to the same document
                        or as a reference when it belongs to another open ``Calc`` document.

                        ``destinationcell``: the destination cell where the copied range of cells will be pasted,
                        as a string. If a range is given, only its top-left cell is considered.
                    Returns
                        A string representing the modified range of cells.
                        The modified area depends on the size of the source area.
            """
            ...

        def CopyToRange(self,
                        sourcerange: Union[RANGE, CALCREFERENCE],
                        destinationrange: RANGE
                        ) -> RANGE:
            """
                Copies downwards and/or rightwards a specified source range (values, formulas and formats)
                to a destination range. The method imitates the behavior of a Copy/Paste operation from
                a source range to a larger destination range.

                - If the height (or width) of the destination area is > 1 row (or column) then the height (or width) of the source must be <= the height (or width) of the destination. Otherwise, nothing happens.
                - If the height (or width) of the destination is = 1 then the destination is expanded downwards (or rightwards) up to the height (or width) of the source range.

                The source range may belong to another open ``Calc`` document.

                Args
                    ``sourcerange``: the source range as a string when it belongs to the same document
                    or as a reference when it belongs to another open ``Calc`` document.

                    ``destinationrange``: the destination of the copied range of cells, as a string.
                Returns
                    A string representing the modified range of cells.
                """
            ...

        def CreateChart(self,
                        chartname: str,
                        sheetname: SHEETNAME,
                        range: RANGE,
                        columnheader: bool = ...,
                        rowheader: bool = ...,
                        ) -> CHART:
            """
                Creates a new chart object showing the data in the specified range.
                The returned chart object can be further manipulated using the ``Chart service``.
                    Args
                        ``chartname``: the user-defined name of the chart to be created.
                        The name must be unique in the same sheet.

                        ``sheetname``: the name of the sheet where the chart will be placed.

                        ``range``: the range to be used as the data source for the chart. The range may refer to
                        any sheet of the ``Calc`` document.

                        ``columnheader``:  when ``True``, the topmost row of the range is used as labels
                        for the category axis or the legend. Defaults to ``False``.

                        ``rowheader``: when ``True``, the leftmost column of the range is used as labels
                        for the category axis or the legend. Defaults to ``False``.
                    Returns
                        A new chart service instance.
                """
            ...

        def CreatePivotTable(self,
                             pivottablename: str,
                             sourcerange: RANGE,
                             targetcell: RANGE,
                             datafields: Union[Sequence[str], str] = ...,
                             rowfields: Union[Sequence[str], str] = ...,
                             columnfields: Union[Sequence[str], str] = ...,
                             filterbutton: bool = ...,
                             rowtotals: bool = ...,
                             columntotals: bool = ...,
                             ) -> RANGE:
            """
                Creates a new pivot table with the properties defined by the arguments passed to the method.

                A name must be provided for the pivot table.
                If a pivot table with the same name already exists in the targeted sheet,
                it will be replaced without warning.

                    Args
                        ``pivottablename``: the user-defined name of the new pivot table.

                        ``sourcerange``: the range containing the raw data, as a string.
                        It is assumed that the first row contains the field names that are used by the pivot table.

                        ``targetcell``: the top-left cell where the new pivot table will be placed.
                        If a range is specified, only its top-left cell is considered.

                        ``datafields``: it can be either a single string or a list of strings
                        that define field names and functions to be applied.
                        When a list is specified, it must follow the syntax ["FieldName[;Function]", ...].
                        The allowed functions are: ``Sum, Count, Average, Max, Min, Product, CountNums, StDev, StDevP, Var, VarP`` and ``Median``.
                        Function names must be provided in English.
                        When all values are numerical, ``Sum`` is the default function, otherwise
                        the default function is ``Count``.

                        ``rowfields``: a single string or a list with the field names that will be used as the pivot
                        table rows.

                        ``columnfields``: a single string or a list with the field names that will be used
                        as the pivot table columns.

                        ``filterbutton``: determines whether a filter button will be displayed above the pivot
                        table (Default = ``True``).

                        ``rowtotals``: specifies if a separate column for row totals will be added to the pivot
                        table (Default = ``True``).

                        ``columntotals``: specifies if a separate row for column totals will be added to the pivot
                        table (Default = ``True``)
                    Returns
                        A string containing the range where the new pivot table was placed.
                """
            ...

        def DAvg(self, range: RANGE) -> float:
            """
                Get the number of the numeric values stored in the given range, excluding values from filtered
                and hidden rows and hidden columns, the same as for the status bar functions.
                    Args
                        ``range``: the range as a string where to get the values from.
                """
            ...

        def DCount(self, range: RANGE) -> int:
            """
                Get the number of numeric values stored in the given range, excluding values from filtered
                and hidden rows and hidden columns, the same as for the status bar functions.
                    Args
                        ``range``: the range as a string where to get the values from.
                """
            ...

        def DMax(self, range: RANGE) -> float:
            """
                Get the greatest of the numeric values stored in the given range, excluding values from filtered
                and hidden rows and hidden columns, the same as for the status bar functions.
                    Args
                        ``range``: the range as a string where to get the values from.
                """
            ...

        def DMin(self, range: RANGE) -> float:
            """
                Get the smallest of the numeric values stored in the given range, excluding values from filtered
                and hidden rows and hidden columns, the same as for the status bar functions.
                    Args
                        ``range``: the range as a string where to get the values from.
                """
            ...

        def DSum(self, range: RANGE) -> float:
            """
                Get the sum of the numeric values stored in the given range, excluding values from filtered
                and hidden rows and hidden columns, the same as for the status bar functions.
                    Args
                        ``range``: the range as a string where to get the values from.
                """
            ...

        def ExportRangeToFile(self,
                              range: RANGE,
                              filename: FILE,
                              imagetype: Literal['pdf', 'jpeg', 'png'] = ...,
                              overwrite: bool = ...,
                              ) -> bool:
            """
                Exports the specified range as an image or PDF file.
                    Args
                        ``range``: a sheet name or a cell range to be exported, as a string.

                        ``filename``: the name of the file to be saved. It must follow the
                        ``SF_FileSystem. FileNaming`` notation.

                        ``imagetype``: identifies the destination file type. Possible values are ``jpeg, pdf``
                        and ``png``. Defaults to ``pdf``.

                        ``overwrite``: when set to ``True``, the destination file may be overwritten.
                        Defaults to ``False``.
                    Returns
                        ``True`` if the destination file was successfully saved.
                """
            ...

        def Forms(self, sheetname: SHEETNAME, form: Union[int, str] = ...) -> Union[FORM, Tuple[str, ...]]:
            """
                Depending on the parameters provided this method will return:

                    - A tuple with the names of all the forms contained in a given sheet (if the form argument is absent)
                    - A ``SFDocuments.Form`` service instance representing the form specified as argument.

                    Args
                        ``sheetname``: the name of the sheet, as a string, from which the form will be retrieved.

                        ``form``: the name or index corresponding to a form stored in the specified sheet.
                        If this argument is absent, the method will return a list with the names of all forms available
                        in the sheet.
                """
            ...
        
        def GetColumnName(self, columnnumber: int) -> str:
            """
                Converts a column number ranging between 1 and 16384 into its corresponding
                letter (column 'A', 'B', ..., 'XFD'). If the given column number is outside
                the allowed range, a zero-length string is returned.
                    Args
                        ``columnnumber``: the column number as an integer value in the interval 1 ... 16384.
                """
            ...

        def GetFormula(self, range: RANGE) -> Union[str, Tuple[str, ...], Tuple[Tuple[str, ...]], ...]:
            """
                Get the formula(s) stored in the given range of cells as a single string, a tuple of strings,
                or a tuple of tuples of strings.
                    Args
                        ``range``: the range where to get the formulas from, as a string.
                    Returns
                        The names of ``Calc`` functions used in the returned formulas are expressed in English.
                """
            ...

        def GetValue(self,
                     range: RANGE
                     ) -> Union[str, Tuple[Union[str, float], ...], Tuple[Tuple[Union[str, float], ...]], ...]:
            """
                Get the value(s) stored in the given range of cells as a single value, a tuple or a tuple of tuples.
                All returned values are either doubles or strings.
                    Args
                        ``range``: the range where to get the values from, as a string.
                    Returns
                        If a cell contains a date, the number corresponding to that date will be returned.
                        To convert numeric values to dates, use the ``CDate()`` function from the
                        ``SFScriptForge.SF_Basic`` service.
                """
            ...

        def ImportFromCSVFile(self,
                              filename: FILE,
                              destinationcell: RANGE,
                              filteroptions: str = ...
                              ) -> RANGE:
            """
                Imports the contents of a CSV-formatted text file and places it on a given destination cell.
                The destination area is cleared of all contents and formats before inserting the contents
                of the CSV file.
                    Args
                        ``filename``: identifies the file to open. It must follow the ``SF_FileSystem.FileNaming``
                        notation.

                        ``destinationcell``: the destination cell to insert the imported data, as a string.
                        If instead a range is given, only its top-left cell is considered.

                        ``filteroptions``: the arguments for the CSV input filter.
                    Returns
                        A string representing the modified range of cells.
                        The modified area depends only on the content of the source file.
                    Note
                        Default ``filteroptions`` make the following assumptions:
                            - The input file encoding is UTF8.
                            - The field separator is a comma, a semicolon or a Tab character.
                            - The string delimiter is the double quote (").
                            - All lines are included.
                            - Quoted strings are formatted as text.
                            - Special numbers are detected.
                            - All columns are presumed to be texts, except if recognized as valid numbers.
                            - The language is English/US, which implies that the decimal separator is "." and the thousands separator is ",".
                """
            ...

        def ImportFromDatabase(self,
                               filename: FILE = ...,
                               registrationname: str = ...,
                               destinationcell: RANGE = ...,
                               sqlcommand: SQL_SELECT = ...,
                               directsql: bool = ...,
                               ) -> None:
            """
                Imports the contents of a database table, query or resultset, i.e. the result of a
                SELECT SQL command, inserting it on a destination cell.

                The destination area is cleared of all contents and formats before inserting the
                imported contents. The size of the modified area is fully determined by the contents in
                the table or query.
                    Args
                        ``filename``: identifies the file to open. It must follow the ``SF_FileSystem.FileNaming``
                        notation.

                        ``registrationname``: the name to use to find the database in the databases register.
                        This argument is ignored if a filename is provided.

                        ``destinationcell``: the destination of the imported data, as a string.
                        If a range is given, only its top-left cell is considered.

                        ``sqlcommand``: a table or query name (without surrounding quotes or square brackets)
                        or a SELECT SQL statement in which table and field names may be surrounded by square brackets
                        or quotes to improve its readability.

                        ``directsql``: when ``True``, the SQL command is sent to the database engine without
                        pre-analysis. Default is ``False``. The argument is ignored for tables.For queries,
                        the applied option is the one set when the query was defined.
                """
            ...

        def ImportStylesFromFile(self,
                                 filename: FILE,
                                 families: Union[str, Sequence[str]] = ...,
                                 overwrite: bool = ...
                                 ):
            """
                This method loads all the styles belonging to one or more style families from a closed
                file into the actual ``Calc`` document.

                Are always imported together:
                    - ``ParagraphStyles`` and ``CharacterStyles``
                    - ``NumberingStyles`` and ``ListStyles``

                Args
                    ``filename``: the file from which to load the styles in the ``SFScriptForge.FileSystem``
                    notation. The file is presumed to be a ``Calc`` document.

                    ``families``: one of the style families present in the actual document,
                    as a case-sensitive string or a tuple of such strings.
                    Leave this argument blank to import all families.

                    ``overwrite``: when ``True``, the actual styles may be overwritten. Default is ``False``.
                Returns
                    ``True`` if styles were successfully imported.

                """
            ...

        def InsertSheet(self, sheetname: SHEETNAME, beforesheet: Union[SHEETNAME, int] = ...) -> bool:
            """
                Inserts a new empty sheet before an existing sheet or at the end of the list of sheets.
                    Args
                        ``sheetname``: the name of the new sheet.

                        ``beforesheet``: the name (string) or index (numeric, starting from 1) of the sheet
                        before which to insert the new sheet. This argument is optional and the default behavior
                        is to insert the sheet at the last position.
                    Returns
                        ``True`` if the sheet could be inserted successfully.
                """
            ...

        def MoveRange(self, source: RANGE, destination: RANGE) -> RANGE:
            """
                Moves a specified source range to a destination cell.
                    Args
                        ``source``: the source range of cells, as a string.

                        ``destination``: the destination cell, as a string. If a range is given,
                        its top-left cell is considered as the destination.
                    Returns
                        A string representing the modified range of cells.
                        The modified area depends only on the size of the source area.
            """
            ...

        def MoveSheet(self,
                      sheetname: SHEETNAME,
                      beforesheet: Union[SHEETNAME, int] = ...
                      ) -> bool:
            """
                Moves an existing sheet and places it before a specified sheet or at the end of the list of sheets.
                    Args
                        ``sheetname``: the name of the sheet to move. The sheet must exist or an exception is raised.

                        ``beforesheet``: The name (string) or index (numeric, starting from 1) of the sheet
                        before which the original sheet will be placed. This argument is optional and
                        the default behavior is to move the sheet to the last position.
                    Returns
                        ``True`` if the sheet could be moved successfully.
                """
            ...

        def Offset(self,
                   range: RANGE,
                   rows: int = ...,
                   columns: int = ...,
                   height: int = ...,
                   width: int = ...,
                   ) -> RANGE:
            """
            Returns a new range (as a string) offset and/or resized by a certain number of rows and columns
            from a given range.

            This method has the same behavior as the homonymous Calc's ``Offset()`` built-in function.

                Args
                    ``range``: the range, as a string, that the method will use as reference to perform
                    the offset operation.

                    ``rows``: the number of rows by which the initial range is offset upwards (negative value)
                    or downwards (positive value). Use 0 (default) to stay in the same row.

                    ``columns``: the number of columns by which the initial range is offset to the left
                    (negative value) or to the right (positive value). Use 0 (default) to stay in the same column.

                    ``height``: the vertical height for an area that starts at the new range position.
                    Omit this argument when no vertical resizing is needed.

                    ``width``: the horizontal width for an area that starts at the new range position.
                    Omit this argument when no horizontal resizing is needed.
                Returns
                    A new range.
                Note
                    - Arguments ``rows`` and ``columns`` must not lead to zero or negative start row or column.
                    - Arguments ``height`` and ``width`` must not lead to zero or negative count of rows or columns.
            """
            ...

        def OpenRangeSelector(self,
                              title: str = ...,
                              selection: RANGE = ...,
                              singlecell: bool = ...,
                              closeafterselect: bool = ...,
                              ) -> RANGE:
            """
                Opens a non-modal dialog that can be used to select a range in the document and returns a string
                containing the selected range.
                    Args
                        ``title``: the title of the dialog, as a string.

                        ``selection``: an initial range that is selected when the dialog is displayed.

                        ``singlecell``: when ``True`` (default) only single-cell selection is allowed.
                        When ``False`` range selection is allowed.

                        ``closeafterselect``: when ``True`` (default) the dialog is closed immediately after
                        the selection is made. When ``False`` the user can change the selection as many times
                        as needed. Dialog closure is manual.
                    Returns
                        The selected range as a string, or the empty string when the user cancelled the request.
                    Note
                        This method opens the same dialog that is used by ``LibreOffice`` when the ``Shrink``
                        button is pressed. For example, the vTools - Goal Seek`` dialog has a ``Shrink`` button
                        to the right of the ``Formula`` cell field.
                """
            ...

        def Printf(self,
                   inputstr: str,
                   range: RANGE,
                   tokencharacter: str = ...
                   ) -> str:
            """
                Returns the input string after substituting its tokens by their values in a given range.

                This method can be used to quickly extract specific parts of a range name, such as the sheet name
                or the first cell column and row, and use them to compose a new range address or a formula.

                    Args
                        ``inputstr``: the string containing the tokens that will be replaced by the corresponding
                        values in ``range``.

                        ``range``: the range from which values will be extracted. If it contains a sheet name,
                        the sheet must exist.

                        ``tokencharacter``: the character used to identify tokens. Defaults to "%".
                    Note
                        Accepted tokens:
                            - ``%S``  - the sheet name containing the range, including single quotes when necessary.
                            - ``%R1`` - the row number of the top left cell of the range.
                            - ``%C1`` - the column letter of the top left cell of the range.
                            - ``%R2`` - the row number of the bottom right cell of the range.
                            - ``%C2`` - the column letter of the bottom right cell of the range.
                    Returns
                        The input string after substitution of the contained tokens.
                """
            ...

        def PrintOut(self,
                     sheetname: SHEETNAME = ...,
                     pages: str = ...,
                     copies: int = ...) -> bool:
            """
                Send the content of the given sheet to the printer.
                The printer might be defined previously by default, by the user or by the ``SF_Document.SetPrinter()``
                method.
                    Args
                        ``sheetname``: the sheet to print, default is the active sheet.

                        ``pages``: the pages to print as a string, like in the user interface. Example: "1-4;10;15-18".
                        Default is all pages.

                        ``copies``: the number of copies. Defaults to 1.
                    Returns
                        ``True`` when successful.
                """
            ...

        def RemoveDuplicates(self,
                             range: RANGE,
                             columns: Union[int | Sequence[int]],
                             header: bool = ...,
                             casesensitive: bool = ...,
                             mode: Literal["CLEAR", "COMPACT"] = ...,
                             ) -> RANGE:
            """
                Removes duplicate rows from a specified range.
                The comparison to determine if a given row is a duplicate is done based on a subset
                of the columns in the range.

                The resulting range replaces the input range, in which, either:

                - all duplicate rows are cleared from their content
                - all duplicate rows are suppressed and rows below are pushed upwards.

                Anyway, the first copy of each set of duplicates is kept and the initial sequence is preserved.

                    Args
                        ``range``: the range, as a string, from which the duplicate rows should be removed.

                        ``columns``: a single integer or a tuple of integers containing column numbers,
                        indicating which columns will be considered to determine if a row is a duplicate or not.
                        If this argument is left blank, then only the first column is used.
                        Column numbers must be in the interval between ``1`` and the range width.

                        ``header``: when ``True``, the first row is a header row. Default is ``False``.

                        ``casesensitive``: for string comparisons. Default is ``False``.

                        ``mode``: either ``"CLEAR"`` or ``"COMPACT"`` (default).
                        For large ranges, the ``"COMPACT"`` mode is probably significantly slower.
                    Returns
                        The resulting range as a string.
                """
            ...

        def RemoveSheet(self, sheetname: SHEETNAME) -> bool:
            """
                Removes an existing sheet from the document.
                    Args
                        ``sheetname``: the name of the sheet to remove.
                    Returns
                        ``True`` if the sheet could be removed successfully.
                """
            ...

        def RenameSheet(self, sheetname: SHEETNAME, newname: SHEETNAME) -> bool:
            """
                Renames the given sheet.
                    Args
                        ``sheetname``: the name of the sheet to rename.

                        ``newname``: the new name of the sheet. It must not exist yet.
                    Returns
                        ``True`` if successful.
                """
            ...

        def SetArray(self,
                     targetcell: RANGE,
                     value: Union[SCALAR, VECTOR, MATRIX]
                     ) -> RANGE:
            """
                Stores the given value starting from a specified target cell. The updated area expands
                itself from the target cell or from the top-left corner of the given range to accommodate
                the size of the input value argument. Vectors are always expanded vertically.
                    Args
                        ``targetcell``: the cell or a range as a string from where to start to store the given value.

                        ``value``: a scalar, a vector or an array  with the new values to be stored from the target
                        cell or from the top-left corner of the range if targetcell is a range.
                        The new values must be strings, numeric values or dates. Other types will cause the
                        corresponding cells to be emptied.
                    Returns
                        A string representing the modified area as a range of cells.
                    Note
                        To dump the full contents of an array in a sheet, use ``SetArray()``. To dump the contents
                        of an array only within the boundaries of the targeted range of cells, use ``SetValue()``.
                """
            ...

        def SetCellStyle(self,
                         targetrange: RANGE,
                         style: str,
                         filterformula: str = ...,
                         filterscope: Literal['CELL', 'ROW', 'COLUMN'] = ...,
                         ) -> RANGE:
            """
                Applies the specified cell style to the given target range.
                The range is updated and the remainder of the sheet is left untouched.
                Either the full range is updated or a selection based on a ``filterformula``.
                If the cell style does not exist, an error is raised.
                    Args
                        ``targetrange``: the range to which the style will be applied, as a string.

                        ``style``: the name of the cell style to apply.

                        ``filterformula``: a ``Calc`` formula that shall be applied to the given range
                        to determine which cells will be affected. The specified formula must return ``True``
                        or ``False``. If this argument is not specified, then all cells in the range are affected.

                        Express the formula in terms of (examples assume a range = ``"A1:J10"``):
                            - the top-left cell of the range when ``filterscope`` = "CELL" e.g. ``"=(A1>100)"``
                            - the topmost row of the range when ``filterscope`` = "ROW" e.g. ``"=(SUM($A1:$J1)<1000)"``
                            - the leftmost column of the range when ``filterscope`` = "COLUMN" e.g. ``"=(A$10=SUM(A$1:A$9))"``

                        ``filterscope``: determines how ``filterformula`` is expanded to the given range.
                       Tuple The argument is mandatory if a ``filterformula`` is specified.
                    Returns
                        A string representing the modified area as a range of cells.
                """
            ...

        def SetFormula(self,
                       targetrange: RANGE,
                       formula: Union[str, Sequence[str], Sequence[Sequence[str]]]
                       ) -> RANGE:
            """
                Inserts the given (list of) formula(s) in the specified range.
                The size of the modified area is equal to the size of the range.
                    Args
                        ``targetrange``: the range to insert the formulas, as a string.

                        ``formula``: a string, a tuple or a tuple of tuples of strings with the new
                        formulas for each cell in the target range.
                    Returns
                        A string representing the modified area as a range of cells.
                    Notes
                        The full range is updated and the remainder of the sheet is left unchanged.

                        If the given ``formula`` is a string, the unique formula is pasted along the
                        whole range with adjustment of the relative references.

                        If the size of ``formula`` is smaller than the size of ``targetrange``, then the
                        remaining cells are emptied.

                        If the size of ``formula`` is larger than the size of ``targetrange``, then the
                        formulas are only partially copied until they fill the size of ``targetrange``.

                        Vectors are always expanded vertically, except if ``targetrange`` has a height
                        of exactly 1 row.
                """
            ...

        def SetValue(self,
                     targetrange: RANGE,
                     value: Union[SCALAR, VECTOR, MATRIX]
                     ) -> RANGE:
            """
                Stores the given value(s) in the specified range. The size of the modified area
                is equal to the size of the target range.
                    Args
                        ``targetrange``: the range where to store the given value, as a string.

                        ``value``: a scalar, a vector or an array with the new values for each
                        cell of the range. The new values must be strings, numeric values or
                        dates. Other types will cause the corresponding cells to be emptied.
                    Returns
                        A string representing the modified area as a range of cells.
                """
            ...
        def ShiftDown(self, range: RANGE, wholerow: bool = ..., rows: int = ...) -> RANGE:
            """
                Moves a given range of cells downwards by inserting empty rows.
                The current selection is not affected.

                Depending on the value of the ``wholerows`` argument the inserted rows can either
                span the width of the specified range or span all columns in the row.
                    Args
                        ``range``: the range above which rows will be inserted, as a string.

                        ``wholerow``: if set to ``False`` (default), then the width
                        of the inserted rows will be the same as the width of the specified range.
                        Otherwise, the inserted row will span all columns in the sheet.

                        ``rows``: the number of rows to be inserted. The default value is
                        the height of the original range. The number of rows must be a positive number.
                    Returns
                        A string representing the new location of the initial range.
                """
            ...

        def ShiftLeft(self, range: RANGE, wholecolumn: bool = ..., columns: int = ...) -> RANGE:
            """
                Deletes the leftmost columns of a given range and moves to the left all cells to the right
                of the given range. The current selection is not affected.

                Depending on the value of the ``wholecolumn`` argument the deleted columns can either span the
                height of the specified range or span all rows in the column.
                    Args
                        ``range``: the range from which cells will be deleted, as a string.

                        ``wholecolumn``: if set to ``False`` (default), then the height of the deleted
                        columns will be the same as the height of the specified range. Otherwise, the deleted
                        columns will span all rows in the sheet.

                        ``columns``: the number of columns to be deleted from the specified range.
                        The default value is the width of the original range, which is also the maximum
                        value of this argument.
                    Returns
                        A string representing the location of the remaining portion of the initial range.
                            If all cells in the original range have been deleted, then an empty string is returned.
                """
            ...

        def ShiftRight(self, range: RANGE, wholecolumn: bool = ..., columns: int = ...) -> RANGE:
            """
                Moves a given range of cells to the right by inserting empty columns.
                The current selection is not affected.

                Depending on the value of the ``wholecolumn`` argument the inserted columns can
                either span the height of the specified range or span all rows in the column.
                    Args
                        ``range``: the range which will have empty columns inserted to its left, as a string.

                        ``wholecolumn``: if set to ``False`` (default), then the height of the inserted
                        columns will be the same as the height of the specified range. Otherwise, the inserted
                        columns will span all rows in the sheet.

                        ``columns``: the number of columns to be inserted. The default value is the width
                        of the original range.
                    Returns
                        A string representing the new location of the initial range.
                    Note
                        If the shifted range exceeds the sheet edges, then nothing happens.
                """
            ...

        def ShiftUp(self, range: RANGE, wholerow: bool = ..., rows: int = ...) -> RANGE:
            """
                Deletes the topmost rows of a given range and moves upwards all cells below the given range.
                The current selection is not affected.

                Depending on the value of the ``wholerow`` argument the deleted rows can either span the width
                of the specified range or span all columns in the row.
                    Args
                        ``range``: the range from which cells will be deleted, as a string.

                        ``wholerow``: if set to ``False`` (default), then the width of the deleted
                        rows will be the same as the width of the specified range. Otherwise, the deleted
                        row will span all columns in the sheet.

                        ``rows``: the number of rows to be deleted from the specified range.
                        The default value is the height of the original range, which is also the maximum
                        value of this argument.
                    Returns
                        A string representing the location of the remaining portion of the initial range.
                        If all cells in the original range have been deleted, then an empty string is returned.
                """
            ...

        def SortRange(self,
                      range: RANGE,
                      sortkeys: Union[int, Sequence[int]],
                      sortorder: Union[Literal['ASC', 'DESC', ''], Sequence[Literal['ASC', 'DESC', '']]] = ...,
                      destinationcell: RANGE = ...,
                      containsheader: bool = ...,
                      casesensitive: bool = ...,
                      sortcolumns: bool = ...,
                      ) -> RANGE:
            """
                Sort the given range on any number of columns/rows. The sorting order may vary by column/row.
                    Args
                        ``range``: the range to be sorted, as a string.

                        ``sortkeys``: a scalar (if 1 column/row) or a tuple of column/row numbers starting from 1.

                        ``sortorder``: a scalar or a tuple of strings containing the values
                        ``"ASC"`` (ascending), ``"DESC"`` (descending) or ``""`` (which defaults to ascending).
                        Each item is paired with the corresponding item in ``sortkeys``.
                        If the sortorder tuple is shorter than ``sortkeys``, the remaining keys are sorted
                        in ascending order.

                        ``destinationcell``: the destination cell of the sorted range of cells,
                        as a string. If a range is given, only its top-left cell is considered.
                        By default, the source range is overwritten.

                        ``containsheader``: when ``True``, the first row/column is not sorted.

                        ``casesensitive``: only for string comparisons. Default is ``False``.

                        ``sortcolumns``:  when ``True``, the columns are sorted from left to right.
                        Default is ``False`` : rows are sorted from top to bottom.
                    Returns
                        A string representing the modified range of cells.
                """
            ...

    # #########################################################################
    # SF_CalcReference CLASS
    # #########################################################################
    class SF_CalcReference(SFServices):
        """
            The SF_CalcReference class has as unique role to hold sheet and range references.
            """

    # #########################################################################
    # SF_Chart CLASS
    # #########################################################################
    class SF_Chart(SFServices):
        """
            The SF_Chart module is focused on the description of chart documents
            stored in Calc sheets.
            With this service, many chart types and chart characteristics available
            in the user interface can be read or modified.
            """

        ChartType: Literal['Pi', 'Bar', 'Donut', 'Column', 'Area', 'Line', 'CY', 'Bubble', 'Net']
        """ Get/set the chart type as a string that can assume one of the following values: "Pie", "Bar", 
        "Donut", "Column", "Area", "Line", "XY", "Bubble", "Net". """
        Deep: bool
        """ When ``True`` indicates that the chart is three-dimensional and each series is arranged in the z-direction.
        When ``False`` series are arranged considering only two dimensions. """
        Dim3D: Union[bool, str]
        """ Get/set if the chart is displayed with 3D elements. If the value is a string, it must be either
        ``"Bar", "Cylinder", "Cone"`` or ``"Pyramid"``. If the boolean ``True`` value is specified,
        then the chart is displayed using 3D bars. """
        Exploded: Union[int, float]
        """ Get/set how much pie segments are offset from the chart center as a percentage of the radius.
        Applicable to ``Pie`` and ``Donut`` charts only. """
        Filled: bool
        """ When ``True``, specifies a filled net chart. Applicable to ``net`` charts only. """
        Legend: bool
        """ Specifies whether the chart has a legend. """
        Percent: bool
        """ When ``True``, chart series are stacked and each category sums up to 100%.
        Applicable to ``Area``, ``Bar``, ``Bubble``, ``Column`` and ``Net`` charts. """
        Stacked: bool
        """ When ``True``, chart series are stacked.
        Applicable to ``Area``, ``Bar``, ``Bubble``, ``Column`` and ``Net`` charts. """
        Title: str
        """ Get/set the main title of the chart. """
        XTitle: str
        """ Get/set the title of the X-axis. """
        YTitle: str
        """ Get/set the title of the Y-axis. """

        XChartObj: UNO
        """ Returns the object representing the chart, which is an instance of the ``ScChartObj`` class. """
        XDiagram: UNO
        """ Returns the ``com.sun.star.chart.XDiagram`` object representing the diagram of the chart. """
        XShape: UNO
        """ Returns the ``com.sun.star.drawing.XShape`` object representing the shape of the chart. """
        XTableChart: UNO
        """ Returns the ``com.sun.star.table.XTableChart`` object representing the data being displayed in the chart.
        """

        def ExportToFile(self,
                         filename: FILE,
                         imagetype: Literal['gif', 'jpeg', 'png', 'svg', 'tiff'] = ...,
                         overwrite: bool = ...
                         ) -> bool:
            """
                Saves the chart as an image file in a specified location.
                    Args
                        ``filename``: identifies the path and file name where the image will be saved.
                        It must follow the notation defined in ``SF_FileSystem.FileNaming``.

                        ``imagetype``: the name of the image type to be created.
                        The following values are accepted: ``gif, jpeg, png`` (default), ``svg`` and ``tiff``.

                        ``overwrite``: specifies if the destination file can be overwritten. Defaults to ``False``.
                    Returns
                        ``True`` if the image file could be successfully created.
                """
            ...

        def Resize(self, xpos: int = ..., ypos: int = ..., width: int = ..., height: int = ...) -> bool:
            """
                Changes the position of the chart in the current sheet and modifies its width and height.
                    Args
                        ``xpos``: specify the new ``X`` position of the chart.

                        ``ypos``: specify the new ``Y`` position of the chart.

                        ``width``: specify the new width of the chart.

                        ``height``: specify the new height of the chart.
                    Returns
                        ``True`` if repositioning/resizing was successful.
                    Note
                        - All arguments are provided as integer values that correspond to ``1/100`` of a millimeter.
                        - Omitted arguments leave the corresponding actual values unchanged.
                        - Negative arguments are ignored.
                """
            ...

    # #########################################################################
    # SF_Form CLASS
    # #########################################################################
    class SF_Form(SFServices):
        """
            Management of forms defined in LibreOffice documents. Supported types are Base, Calc and Writer documents.
            It includes the management of sub-forms
            Each instance of the current class represents a single form or a single sub-form
            A form may optionally be (understand "is often") linked to a data source manageable with
            the SFDatabases.Database service. The current service offers rapid access to that service.
            """

        AllowDeletes: bool
        """ Get/set if the form allows to delete records. """
        AllowInserts: bool
        """ Get/set if the form allows to add records. """
        AllowUpdates: bool
        """ Get/set if the form allows to update records. """
        BaseForm: str
        """ Get the hierarchical name of the Base form document containing the actual form. """
        Bookmark: SCALAR
        """ Specifies uniquely the current record of the form's underlying table, query or SQL statement. """
        CurrentRecord: int
        """ Identifies the current record in the dataset being viewed on a form. If the row number is positive,
        the cursor moves to the given row number with respect to the beginning of the result set. Row count starts at 1.
        If the given row number is negative, the cursor moves to an absolute row position with respect to the end
        of the result set. Row -1 refers to the last row in the result set. """
        Filter: str
        """ Specifies a subset of records to be displayed as a ``SQL WHERE``-clause without the ``WHERE`` keyword. """
        LinkChildFields: Tuple[str, ...]
        """ Specifies how records in a child subform are linked to records in its parent form. """
        LinkParentFields: Tuple[str, ...]
        """ Specifies how records in a child subform are linked to records in its parent form. """
        Name: str
        """ The name of the current form. """

        OnApproveCursorMove: SCRIPT_URI
        """ Get/set the macro triggered by the ``Before record change`` event."""
        OnApproveParameter: SCRIPT_URI
        """ Get/set the macro triggered by the ``Fill parameters`` event."""
        OnApproveReset: SCRIPT_URI
        """ Get/set the macro triggered by the ``Prior to reset`` event."""
        OnApproveRowChange: SCRIPT_URI
        """ Get/set the macro triggered by the ``Before record action`` event."""
        OnApproveSubmit: SCRIPT_URI
        """ Get/set the macro triggered by the ``Before submitting`` event."""
        OnConfirmDelete: SCRIPT_URI
        """ Get/set the macro triggered by the ``Confirm deletion`` event."""
        OnCursorMoved: SCRIPT_URI
        """ Get/set the macro triggered by the ``After record change`` event."""
        OnErrorOccurred: SCRIPT_URI
        """ Get/set the macro triggered by the ``Error occurred`` event."""
        OnLoaded: SCRIPT_URI
        """ Get/set the macro triggered by the ``When loading`` event."""
        OnReloaded: SCRIPT_URI
        """ Get/set the macro triggered by the ``When reloading`` event."""
        OnReloading: SCRIPT_URI
        """ Get/set the macro triggered by the ``Before reloading`` event."""
        OnResetted: SCRIPT_URI
        """ Get/set the macro triggered by the ``After resetting`` event."""
        OnRowChanged: SCRIPT_URI
        """ Get/set the macro triggered by the ``After record action`` event."""
        OnUnloaded: SCRIPT_URI
        """ Get/set the macro triggered by the ``When unloading`` event."""
        OnUnloading: SCRIPT_URI
        """ Get/set the macro triggered by the ``Before unloading`` event."""

        OrderBy: str
        """ Specifies in which order the records should be displayed as a ``SQL ORDER BY`` clause
        without the ``ORDER BY`` keywords. """
        Parent: Union[FORM, FORMDOCUMENT, DOCUMENT, CALC, WRITER]
        """ The parent of the current form. It can be either a ``SFDocuments.Form``, a ``SFDocuments.FormDocument``,
        a ``SFDocuments.Document`` object or one of its subclasses. """
        RecordSource: Union[SQL_SELECT, str]
        """ Specifies the source of the data, as a table name, a query name or a SQL statement. """

        XForm: UNO
        """ The UNO object representing interactions with the form. Refer to ``com.sun.star.form.XForm`` and
        ``com.sun.star.form.component.DataForm`` in the API documentation for detailed information. """

        def Activate(self) -> bool:
            """
                Sets the focus on the container of the current ``Form`` instance.

                The behavior of the ``Activate`` method depends on the type of document where the form is located.
                    * In `Writer`` documents: Sets the focus on that document.
                    * In ``Calc`` documents: Sets the focus on the sheet to which the form belongs.
                    * In ``Base`` documents: Sets the focus on the ``FormDocument`` the ``Form`` refers to.
                Returns
                    ``True`` if focusing was successful.
                """
            ...

        def CloseFormDocument(self) -> bool:
            """
                Closes the ``form`` document containing the actual ``Form`` instance. The ``Form`` instance is disposed.

                The method is deprecated. Use the ``SF_FormDocument.CloseDocument()`` method instead.

                    Returns
                        ``True`` if closure is successful.
                    Note
                        This method only closes form documents located in ``Base`` documents.
                        If the form is stored in a ``Writer`` or ``Calc`` document, calling ``CloseFormDocument()``
                        will have no effect.
                """
            ...

        def Controls(self, controlname: str = ...) -> Union[FORMCONTROL, Tuple[str, ...]]:
            """
                The value returned by the method depends on the arguments provided:
                    - If the method is called without arguments, then it returns the list of the controls contained in the form.

                    - If the optional ``controlName`` argument is provided, the method returns a ``FormControl`` class instance referring to the specified control.

                Note
                    Subform controls are ignored by this method.
                """
            ...

        def GetDatabase(self, user: str = ..., password: str = ...) -> Optional[DATABASE]:
            """
                Gets a ``SFDatabases.Database`` instance giving access to the execution of SQL commands
                on the database the current form is connected to and/or that is stored in the current ``Base`` document.

                Each form has its own database connection, except in ``Base`` documents where they all
                share the same connection.
                    Args
                        ``user``, ``password````: the login parameters.
                    Returns
                        A ``SFDatabases.Database``  class instance.
                """
            ...

        def MoveFirst(self) -> bool:
            """
                The form cursor is positioned on the first record.

                Returns
                    ``True`` if successful.
                """
            ...

        def MoveLast(self) -> bool:
            """
                The form cursor is positioned on the last record.

                Returns
                    ``True`` if successful.
                """
            ...

        def MoveNew(self) -> bool:
            """
                The form cursor is positioned on the new record area.

                Returns
                    ``True`` if successful.
                """
            ...

        def MoveNext(self, offset: int = ...) -> bool:
            """
                The form cursor is positioned on the next record.
                    Args
                        ``offset``: the number of records to go forward. Defaults to 1.
                    Returns
                        ``True`` if successful.
                """
            ...

        def MovePrevious(self, offset: int = ...) -> bool:
            """
                The form cursor is positioned on the previous record.
                    Args
                        ``offset``: the number of records to go backward. Defaults to 1.
                    Returns
                        ``True`` if successful.
                """
            ...

        def Requery(self) -> bool:
            """
                Reloads the current data from the database and refreshes the form.
                The cursor is positioned on the first record.

                Returns
                    ``True`` if successful.
                """
            ...

        def Subforms(self, subform: Union[int, str] = ...) -> Union[FORM, Tuple[str, ...]]:
            """
                Depending on the parameters provided this method will return:

                    - A tuple with the names of all the subforms contained in the actual form (if the form argument is absent)
                    - A ``SFDocuments.Form`` service instance representing the subform specified as argument.

                    Args
                        ``subform``: the name or index corresponding to a subform stored in the actual form.
                        If this argument is absent, the method will return a list with the names of
                        all available subforms.
                """
            ...

    # #########################################################################
    # SF_FormControl CLASS
    # #########################################################################
    class SF_FormControl(SFServices):
        """
            Manage the controls belonging to a form or subform stored in a document.

            Each instance of the current class represents a single control within a form, a subform or a column
            in a table-control. A prerequisite is that all controls within the same form, subform or table-control
            must have a unique name.
            """

        Action: Literal['none', 'submitForm', 'resetForm', 'refreshForm', 'moveToFirst', 'moveToLast', 'moveToNext',
        'moveToPrev', 'saveRecord', 'moveToNew', 'deleteRecord', 'undoRecord']
        """ Get/set the action triggered when the button is clicked. Applicable to ``Button`` controls.  """
        Caption: str
        """ Get/set the text associated with the control. Applicable to ``Button, CheckBox, FixedText, 
        GroupBox, RadioButton`` controls.  """
        ControlSource: str
        """ Specifies the rowset field mapped onto the current control. Applicable to
        ``CheckBox, ComboBox, CurrencyField, DateField, FormattedField, ImageControl, ListBox, NumericField, 
        PatternField, RadioButton, TextField, TimeField`` controls. """
        ControlType: str
        """ get the type of control as a string.    """
        Default: bool
        """ Get/set whether a command button is the default (OK) button.
        Applicable to ``Button`` controls.  """
        DefaultValue: bool
        """ Specifies the default value used to initialize a control in a new record.
        Applicable to ``CheckBox, ComboBox, CurrencyField, DateField, FileControl, FormattedField, ListBox, 
        NumericField, PatternField, RadioButton, SpinButton, TextField, TimeField`` controls. """
        Enabled: bool
        """ Specifies if the control is accessible with the cursor. """
        Format: Literal["Standard (short)", "Standard (short YY)", "Standard (short YYYY)", "Standard (long)",
        "DD/MM/YY", "MM/DD/YY", "YY/MM/DD", "DD/MM/YYYY", "MM/DD/YYYY", "YYYY/MM/DD", "YY-MM-DD", "YYYY-MM-DD",
        "24h short", "24h long", "12h short", "12h long"]
        """ Get/set the format used to display dates and times. Applicable to ``DateField, TimeFiels, FormattedField``
        controls.   """
        ListCount: int
        """ Get the number of rows in the control. Applicable to ``ComboBox, ListBox`` controls.  """
        ListIndex: int
        """ Get/set which item is selected  in the control. In case of multiple selection, the index of the first
        item is returned. Applicable to ``ComboBox, ListBox`` controls.  """
        ListSource: Union[str, Tuple[str, ...]]
        """ Specifies the data contained in a control as a tuple of string values.
        Combined with ``ListSourceType``, may also contain the name of a table, a query or a complete SQL statement.
        Applicable to ``ComboBox, ListBox`` controls. """
        ListSourceType: int
        """ Specifies the type of data contained in a control. It must be one of
        the ``com.sun.star.form.ListSourceType.*`` constants. Applicable to ``ComboBox, ListBox`` controls. """
        Locked: bool
        """ Get/set  if the control is read-only.. Applicable to ``ComboBox, CurrencyField, DateField,  FileControl, 
        FormattedField, ImageControl, ListBox, NumericField, PatternField, TextField, TimeField`` controls.   """
        MultiSelect: bool
        """ Get/set whether a user can make multiple selections in a ``listbox``.   """
        Name: str
        """ The name of the control.    """

        OnActionPerformed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Execute action`` event. """
        OnAdjustmentValueChanged: SCRIPT_URI
        """ Get/set the macro triggered by the ``While adjusting`` event. """
        OnApproveAction: SCRIPT_URI
        """ Get/set the macro triggered by the ``Approve action`` event. """
        OnApproveReset: SCRIPT_URI
        """ Get/set the macro triggered by the ``Prior to reset`` event. """
        OnApproveUpdate: SCRIPT_URI
        """ Get/set the macro triggered by the ``WBefore updating`` event. """
        OnChanged: SCRIPT_URI
        """ Get/set the macro triggered by the ``Changed`` event. """
        OnErrorOccurred: SCRIPT_URI
        """ Get/set the macro triggered by the ``Error occurred`` event. """
        OnFocusGained: SCRIPT_URI
        """ Get/set the macro triggered by the ``When receiving focus`` event."""
        OnFocusLost: SCRIPT_URI
        """ Get/set the macro triggered by the ``When losing focus`` event."""
        OnItemStateChanged: SCRIPT_URI
        """ Get/set the macro triggered by the ``Item status changed`` event. """
        OnKeyPressed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Key pressed`` event."""
        OnKeyReleased: SCRIPT_URI
        """ Get/set the macro triggered by the ``Key released`` event."""
        OnMouseDragged: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse moved while button pressed`` event."""
        OnMouseEntered: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse inside`` event."""
        OnMouseExited: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse outside`` event."""
        OnMouseMoved: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse moved`` event."""
        OnMousePressed: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse button pressed`` event."""
        OnMouseReleased: SCRIPT_URI
        """ Get/set the macro triggered by the ``Mouse button released`` event."""
        OnResetted: SCRIPT_URI
        """ Get/set the macro triggered by the ``After resetting`` event. """
        OnTextChanged: SCRIPT_URI
        """ Get/set the macro triggered by the ``Text modified`` event. """
        OnUpdated: SCRIPT_URI
        """ Get/set the macro triggered by the ``After updating`` event. """

        Parent: Union[FORM, FORMCONTROL]
        """ Depending on the parent type, a form, a subform or a tablecontrol, returns the parent 
        ``SFDocuments.SF_Form`` or ``SFDocuments.SF_FormControl`` class instance. """
        Picture: FILE
        """ Get/se the file name (in ``FileSystem.FileNaming`` notation) containing a bitmap or other type of graphic
        to be displayed on the control. Applicable to ``Button, ImageButton, ImageControl`` controls. """
        Required: bool
        """ A control is said required when the underlying data must not contain a ``NULL`` value.
         Applicable to ``CheckBox, ComboBox, CurrencyField, DateField, ListBox, NumericField, PatternField, 
         RadioButton, SpinButton, TextField, TimeField`` controls."""
        Text: str
        """ Get the text being displayed by the control. Applicable to ``ComboBox, DateField, FileControl,
        FormattedField, PatternField, TextField, TimeField`` controls.  """
        TipText: str
        """ Specifies the text that appears as a tooltip when you hover the mouse pointer over the control. """
        TripleState: bool
        """ Get/set whether the ``checkbox`` control may appear dimmed (grayed).    """
        Value: Any
        """
        Get/set the content of the control:

        - ``Button``: bool - For toggle buttons only.
        - ``CheckBox``: bool, int - 0, ``False``: not checked, 1, ``True``: checked, 2: grayed, don't know.
        - ``ComboBox``: str - The selected value. The ``ListIndex`` property is an alternate option.
        - ``CurrencyField``: int, float.
        - ``DateField``: datetime.datetime.
        - ``FileControl``: FILE.
        - ``FormattedField``: str, int, float.
        - ``HiddenControl``: str,
        - ``ListBox``: List(str), str - Get the selected row(s) as a single string or an array of strings. Only a single value can be set.
        - ``NumericField``: int, float.
        - ``PatternField``: str.
        - ``RadioButton``: bool - Each button has its own name. Multiple RadioButton controls are linked together when they share the same group name. If a radiobutton is set to ``True``, the other related buttons are automatically set to ``False``.
        - ``ScrollBar``: int - Must be within the predefined bounds.
        - ``SpinButton``: int - Must be within the predefined bounds.
        - ``TextField``: str - The text appearing in the control.
        - ``TimeField``: datetime.datetime.
        Not applicable to ``FixedText, GroupBox, ImageButton, ImageControl, NavigationBar`` and ``TableControl``
        form controls.
        """
        Visible: bool
        """ Get/set if the form control is hidden or visible.  """

        XControlModel: UNO
        """ The UNO representation (``com.sun.star.awt.XControlModel``) of the control model.        """
        XControlView: UNO
        """ The UNO representation (``com.sun.star.awt.XControl``) of the control view.  """

        def Controls(self, controlname: str = ...) -> Union[FORMCONTROL, Tuple[str, ...]]:
            """
                This method is applicable only to controls of the ``TableControl`` type.
                The returned value depends on the arguments provided.

                If ``controlname`` is absent, then a tuple containing the names of all controls is returned.

                If ``controlname`` is provided, the method returns a ``FormControl`` class instance
                corresponding to the specified control.

                    Args
                        ``controlname``: a valid control name as a case-sensitive string.
                    Returns
                        The list of available control names
                        or an instance of the ``SFDocuments.SF_FormControl`` class.
                """
            ...

        def SetFocus(self) -> bool:
            """
                Sets the focus on the control.

                Returns
                    ``True`` if focusing was successful.
                """
            ...

    # #########################################################################
    # SF_FormDocument CLASS
    # #########################################################################
    class SF_FormDocument(SF_Document, SFServices):
        """
            The ``FormDocument`` service allows to access form documents stored in ``Base`` documents.

            Each form document may be composed of one or more forms, including the main form and other sub-forms.

            This service inherits methods and properties from the ``Document`` service and is often used alongside
            the ``Base`` and ``Database`` services.
            """

        def CloseDocument(self) -> bool:
            """
                Close the form document and dispose the actual instance.

                Returns
                    ``True`` if closure is successful.
                """
            ...

        def Forms(self, form: Union[int, str] = ...) -> Union[FORM, Tuple[str, ...]]:
            """
                Depending on the parameters provided this method will return:

                    - A tuple with the names of all the forms contained in the form document (if the form argument is absent)
                    - A ``SFDocuments.Form`` service instance representing the form specified as argument.

                    Args
                        ``form``: the name or index corresponding to a form stored in the form document.
                        If this argument is absent, the method will return a list with the names of all available forms.
                """
            ...

        def GetDatabase(self, user: str = ..., password: str = ...) -> Optional[DATABASE]:
            """
                Returns a Database service instance (service = ``SFDatabases.Database``) giving access
                to the execution of SQL commands on the database linked with the actual form document.
                    Args
                        ``user``, ``oassword``: the login parameters.
                """
            ...

        def PrintOut(self,
                     pages: str = ...,
                     copies: int = ...,
                     printbackground: bool = ...,
                     printblankpages: bool = ...,
                     printevenpages: bool = ...,
                     printoddpages: bool = ...,
                     printimages: bool = ...,
                     ) -> bool:
            """
                Send the content of the document to the printer.

                The printer might be defined previously by default, by the user or by the SetPrinter() method.

                    Args
                        ``pages``: the pages to print as a string, like in the user interface.
                        Example: "1-4;10;15-18". Defaults to all pages.

                        ``copies``:  the number of copies to print. Defaults to 1.

                        ``printbackground``: print the background image when ``True`` (default).

                        ``printblankpages``: when ``False`` (default), omit empty pages.

                        ``printevenpages``: print the left pages when ``True`` (default).

                        ``printoddpages``: print the right pages when ``True`` (default).

                        ``printimages``: print the graphic objects when ``True`` (defauly).
                    Returns
                         ``True`` when successful.
                """
            ...

    # #########################################################################
    # SF_Writer CLASS
    # #########################################################################
    class SF_Writer(SF_Document, SFServices):
        """
            Many methods are generic for all types of documents and are inherited from the ``SF_Document``
            module, whereas other methods that are specific for ``Writer`` documents are defined
            in the ``SF_Writer`` module.
            """

        def Forms(self, form: Union[int, str] = ...) -> Union[FORM, Tuple[str, ...]]:
            """
                Depending on the parameters provided this method will return:

                    - A tuple with the names of all the forms contained in the writer document (if the form argument is absent)
                    - A ``SFDocuments.Form`` service instance representing the form specified as argument.

                    Args
                        ``form``: the name or index corresponding to a form stored in the form document.
                        If this argument is absent, the method will return a list with the names of all available forms.
                """
            ...

        def ImportStylesFromFile(self,
                                 filename: FILE,
                                 families: Union[str, Sequence[str]] = ...,
                                 overwrite: bool = ...
                                 ):
            """
                This method loads all the styles belonging to one or more style families from a closed
                file into the actual ``Writer`` document.

                Are always imported together:
                    - ``ParagraphStyles`` and ``CharacterStyles``
                    - ``NumberingStyles`` and ``ListStyles``

                Args
                    ``filename``: the file from which to load the styles in the ``SFScriptForge.FileSystem``
                    notation. The file is presumed to be a ``Writer`` document.

                    ``families``: one of the style families present in the actual document,
                    as a case-sensitive string or a tuple of such strings.
                    Leave this argument blank to import all families.

                    ``overwrite``: when ``True``, the actual styles may be overwritten. Default is ``False``.
                Returns
                    ``True`` if styles were successfully imported.

                """
            ...

        def PrintOut(self,
                     pages: str = ...,
                     copies: int = ...,
                     printbackground: bool = ...,
                     printblankpages: bool = ...,
                     printevenpages: bool = ...,
                     printoddpages: bool = ...,
                     printimages: bool = ...,
                     ) -> bool:
            """
                Send the content of the document to the printer.

                The printer might be defined previously by default, by the user or by the SetPrinter() method.

                    Args
                        ``pages``: the pages to print as a string, like in the user interface.
                        Example: "1-4;10;15-18". Defaults to all pages.

                        ``copies``:  the number of copies to print. Defaults to 1.

                        ``printbackground``: print the background image when ``True`` (default).

                        ``printblankpages``: when ``False`` (default), omit empty pages.

                        ``printevenpages``: print the left pages when ``True`` (default).

                        ``printoddpages``: print the right pages when ``True`` (default).

                        ``printimages``: print the graphic objects when ``True`` (defauly).
                    Returns
                         ``True`` when successful.
                """
            ...

class SFWidgets:
    """
    The SFWidgets class manages toolbars, menus and popup menus.
    """

    # #########################################################################
    # SF_Menu CLASS
    # #########################################################################
    class SF_Menu(SFServices):
        """
            Display a menu in the menubar of a document or a form document.
            After use, the menu will not be saved neither in the application settings, nor in the document.

            The menu will be displayed, as usual, when its header in the menubar is clicked.
            When one of its items is selected, there are 3 alternative options:
                - a UNO command (like ".uno:About") is triggered
                - a user script is run receiving a standard argument defined in this service
                - one of above combined with a toggle of the status of the item

            The menu is described from top to bottom. Each menu item receives a numeric and a string identifier.

            The ``SF_Menu`` service provides the following capabilities:
                - Creation of menus with custom entries, checkboxes, radio buttons and separators.
                - Decoration of menu items with icons and tooltips.
            Menu entries associated with a script receive a comma-separated string argument with the following values:
                - The toplevel name of the menu.
                - The string ID of the selected menu entry.
                - The numeric ID of the selected menu entry.
                - The current state of the menu item. This is useful for checkboxes and radio buttons. If the item is checked, the value "1" is returned, otherwise "0" is returned.
            """

        ShortcutCharacter: str
        """ Character used to define the access key of a menu item. The default character is "~" (tilde). """
        SubmenuCharacter: str
        """ Character or string that defines how menu items are nested. The default character is ">".
        
        Example
            ``oMenu.AddItem("Item A")``
            
            ``oMenu.AddItem("Item B>Item B.1")``
            
            ``oMenu.AddItem("Item B>Item B.2")``
            
            ``oMenu.AddItem("---")``
            
            ``oMenu.AddItem("Item C>Item C.1>Item C.1.1")``
            
            ``oMenu.AddItem("Item C>Item C.1>Item C.1.2")``
            
            ``oMenu.AddItem("Item C>Item C.2>Item C.2.1")``
            
            ``oMenu.AddItem("Item C>Item C.2>Item C.2.2")``
            
            ``oMenu.AddItem("Item C>Item C.2>---")``
            
            ``oMenu.AddItem("Item C>Item C.2>Item C.2.3")``
            
            ``oMenu.AddItem("Item C>Item C.2>Item C.2.4")``
        """

        def AddCheckBox(self,
                        menuitem: str,
                        name: str = ...,
                        status: bool = ...,
                        icon: str = ...,
                        tooltip: str = ...,
                        command: str = ...,
                        script: SCRIPT_URI = ...,
                        ) -> int:
            """
                Inserts a checkbox in the menu.
                    Args
                        ``menuitem``: defines the text to be displayed in the menu.
                        This argument also defines the hierarchy of the item inside the menu by using the submenu
                        character.

                        ``name``: the string value used to identify the menu item.
                        By default, the last component of the menu hierarchy is used.

                        ``status``: defines whether the item is selected when the menu is created. Defaults to ``False``.

                        ``icon``: the relative path and name of the icon to be displayed versus the folder structure
                        found in the ZIP files located in the ``$INSTALLDIR/share/config`` directory.
                        The actual icon shown depends on the icon set being used.

                        ``tooltip``: text to be displayed as tooltip.

                        ``command``: the name of a UNO command without the .uno: prefix. If the command name
                        does not exist, nothing will happen when the item is clicked.

                        ``script``: the URI for a Basic or Python script that will be executed when the item is clicked.
                    Note
                        The ``command`` and ``script`` arguments are mutually exclusive.
                    Returns
                        A numeric value that uniquely identifies the inserted item.
                """
            ...

        def AddItem(self,
                    menuitem: str,
                    name: str = ...,
                    icon: str = ...,
                    tooltip: str = ...,
                    command: str = ...,
                    script: SCRIPT_URI = ...,
                    ) -> int:
            """
                Inserts a label entry in the menu.
                    Args
                        ``menuitem``: defines the text to be displayed in the menu.
                        This argument also defines the hierarchy of the item inside the menu by using the submenu
                        character.

                        ``name``: the string value used to identify the menu item.
                        By default, the last component of the menu hierarchy is used.

                        ``icon``: the relative path and name of the icon to be displayed versus the folder structure
                        found in the ZIP files located in the ``$INSTALLDIR/share/config`` directory.
                        The actual icon shown depends on the icon set being used.

                        ``tooltip``: text to be displayed as tooltip.

                        ``command``: the name of a UNO command without the .uno: prefix. If the command name
                        does not exist, nothing will happen when the item is clicked.

                        ``script``: the URI for a Basic or Python script that will be executed when the item is clicked.
                    Note
                        The ``command`` and ``script`` arguments are mutually exclusive.
                    Returns
                        A numeric value that uniquely identifies the inserted item.
                """
            ...

        def AddRadioButton(self,
                           menuitem: str,
                           name: str = ...,
                           status: bool = ...,
                           icon: str = ...,
                           tooltip: str = ...,
                           command: str = ...,
                           script: SCRIPT_URI = ...,
                           ) -> int:
            """
                Inserts a radio btton in the menu.
                    Args
                        ``menuitem``: defines the text to be displayed in the menu.
                        This argument also defines the hierarchy of the item inside the menu by using the submenu
                        character.

                        ``name``: the string value used to identify the menu item.
                        By default, the last component of the menu hierarchy is used.

                        ``status``: defines whether the item is selected when the menu is created. Defaults to ``False``.

                        ``icon``: the relative path and name of the icon to be displayed versus the folder structure
                        found in the ZIP files located in the ``$INSTALLDIR/share/config`` directory.
                        The actual icon shown depends on the icon set being used.

                        ``tooltip``: text to be displayed as tooltip.

                        ``command``: the name of a UNO command without the .uno: prefix. If the command name
                        does not exist, nothing will happen when the item is clicked.

                        ``script``: the URI for a Basic or Python script that will be executed when the item is clicked.
                    Note
                        The ``command`` and ``script`` arguments are mutually exclusive.
                    Returns
                        A numeric value that uniquely identifies the inserted item.
                """
            ...

    # #########################################################################
    # SF_Menu CLASS
    # #########################################################################
    class SF_PopupMenu(SFServices):
        """
            Display a popup menu anywhere and any time.
            A popup menu is usually triggered by a mouse action (typically a right-click) on a dialog, a form
            or one of their controls. In this case the menu will be displayed below the clicked area.

            When triggered by other events, including in the normal flow of a user script, the script should
            provide the coordinates of the top-left edge of the menu versus the actual component.

            The menu is described from top to bottom. Each menu item receives a numeric and a string identifier.
            The ``Execute()`` method returns the item selected by the user.

            The ``SF_PopupMenu`` service provides the following capabilities:
                - Creation of menus with custom entries, checkboxes, radio buttons and separators.
                - Decoration of menu items with icons and tooltips.
            """

        ShortcutCharacter: str
        """ Character used to define the access key of a menu item. The default character is "~" (tilde). """
        SubmenuCharacter: str
        """ Character or string that defines how menu items are nested. The default character is ">".

        Example
            ``oMenu.AddItem("Item A")``

            ``oMenu.AddItem("Item B>Item B.1")``

            ``oMenu.AddItem("Item B>Item B.2")``

            ``oMenu.AddItem("---")``

            ``oMenu.AddItem("Item C>Item C.1>Item C.1.1")``

            ``oMenu.AddItem("Item C>Item C.1>Item C.1.2")``

            ``oMenu.AddItem("Item C>Item C.2>Item C.2.1")``

            ``oMenu.AddItem("Item C>Item C.2>Item C.2.2")``

            ``oMenu.AddItem("Item C>Item C.2>---")``

            ``oMenu.AddItem("Item C>Item C.2>Item C.2.3")``

            ``oMenu.AddItem("Item C>Item C.2>Item C.2.4")``
        """

        def AddCheckBox(self,
                        menuitem: str,
                        name: str = ...,
                        status: bool = ...,
                        icon: str = ...,
                        tooltip: str = ...,
                        ) -> int:
            """
                Inserts a checkbox in the menu.
                    Args
                        ``menuitem``: defines the text to be displayed in the menu.
                        This argument also defines the hierarchy of the item inside the menu by using the submenu
                        character.

                        ``name``: the string value used to identify the menu item.
                        By default, the last component of the menu hierarchy is used.

                        ``status``: defines whether the item is selected when the menu is created. Defaults to ``False``.

                        ``icon``: the relative path and name of the icon to be displayed versus the folder structure
                        found in the ZIP files located in the ``$INSTALLDIR/share/config`` directory.
                        The actual icon shown depends on the icon set being used.

                        ``tooltip``: text to be displayed as tooltip.
                    Returns
                        A numeric value that uniquely identifies the inserted item.
                """
            ...

        def AddItem(self,
                    menuitem: str,
                    name: str = ...,
                    icon: str = ...,
                    tooltip: str = ...,
                    ) -> int:
            """
                Inserts a label entry in the menu.
                    Args
                        ``menuitem``: defines the text to be displayed in the menu.
                        This argument also defines the hierarchy of the item inside the menu by using the submenu
                        character.

                        ``name``: the string value used to identify the menu item.
                        By default, the last component of the menu hierarchy is used.

                        ``icon``: the relative path and name of the icon to be displayed versus the folder structure
                        found in the ZIP files located in the ``$INSTALLDIR/share/config`` directory.
                        The actual icon shown depends on the icon set being used.

                        ``tooltip``: text to be displayed as tooltip.
                    Returns
                        A numeric value that uniquely identifies the inserted item.
                """
            ...

        def AddRadioButton(self,
                           menuitem: str,
                           name: str = ...,
                           status: bool = ...,
                           icon: str = ...,
                           tooltip: str = ...,
                           ) -> int:
            """
                Inserts a radio btton in the menu.
                    Args
                        ``menuitem``: defines the text to be displayed in the menu.
                        This argument also defines the hierarchy of the item inside the menu by using the submenu
                        character.

                        ``name``: the string value used to identify the menu item.
                        By default, the last component of the menu hierarchy is used.

                        ``status``: defines whether the item is selected when the menu is created. Defaults to ``False``.

                        ``icon``: the relative path and name of the icon to be displayed versus the folder structure
                        found in the ZIP files located in the ``$INSTALLDIR/share/config`` directory.
                        The actual icon shown depends on the icon set being used.

                        ``tooltip``: text to be displayed as tooltip.
                    Returns
                        A numeric value that uniquely identifies the inserted item.
                """
            ...

        def Execute(self, returnid: bool = ...) -> Union[int, str]:
            """
                Displays the popup menu and waits for a user action.
                    Args
                        ``returnid``: if ``True`` (default), the numeric identfier of the selected item is returned.
                        If ``False``, the method returns the item's name.
                    Returns
                        The item clicked by the user.
                        If the user clicks outside the popup menu or presses the ``Esc`` key, then no item is selected.
                        In such cases, the returned value is either ``0`` (zero) or the empty string, depending
                        on the ``returnid`` argument.
                """
            ...

    # #########################################################################
    # SF_Toolbar CLASS
    # #########################################################################
    class SF_Toolbar(SFServices):
        """
            Each component has its own set of toolbars, depending on the component type
            (Calc, Writer, Basic IDE, ...).

            In the context of the actual class, a toolbar is presumed defined statically:
                - either bat LibreOffice installation,
                - or by a customization done by the user at application or at document levels.

            The ``Toolbar`` service allows to retrieve information related to the toolbars available for
            a specific document window.

            With this service it is possible to:
                - toggle the visibility of specific toolbars.
                - access to information about the buttons available in each toolbar.

            This service handles both built-in and custom toolbars.
            """

        BuiltIn: bool
        """ Returns ``True`` when the toolbar is part of the set of standard toolbars shipped with LibreOffice. """
        Docked: bool
        """ Returns ``True`` when the toolbar is active in the window and docked. """
        HasGlobalScope: bool
        """ Returns ``True`` when the toolbar is available in all documents of the same type. """
        Name: str
        """ Returns the name of the toolbar. """
        ResourceURL: str
        """ Returns the resource URL of the toolbar, in the form ``"private:toolbar/toolbar_name"``. """
        Visible: bool
        """ Get/set whether the toolbar is active and visible in the document window. """
        XUIElement: UNO
        """ Returns the ``com.sun.star.ui.XUIElement`` UNO object that represents the toolbar. """

        def ToolbarButtons(self, buttonname: str = ...) -> Union[TOOLBARBUTTON, Tuple[str]]:
            """
                Returns either a list of the available toolbar button names in the actual toolbar
                or a ``ToolbarButton`` object instance.
                    Args
                        ``buttonname``: the usual name of one of the available buttons in the actual toolbar.
                        The button name is the localized button name defined in the ``Tools - Customize - Toolbars``
                        dialog.
                    Returns
                        A tuple of button names when the argument is absent,
                        or a new ``SF_ToolbarButton`` object instance otherwise.
                """
            ...

    # #########################################################################
    # SF_Toolbar CLASS
    # #########################################################################
    class SF_ToolbarButton(SFServices):
        """
            A toolbar consists in a series of graphical controls to trigger actions.
            The ``Toolbar`` service gives access to the ``ToolbarButton`` service to manage
            the individual buttons belonging to the toolbar.

            With this service it is possible to:
                - know the characteristics of the button
                - toggle the visibility of the button.
                - execute the command associated with the toolbar button.
            """

        Caption: str
        """ Returns the name of the button. """
        Height: int
        """ Returns the height of the button, in pixels. """
        Index: int
        """ Returns the index of the button in its parent toolbar. """
        OnClick: str
        """ Get/set the UNO command or the script executed when the button is pressed. """
        Parent: TOOLBAR
        """ Returns a ``Toolbar`` service instance corresponding to the parent toolbar of the current
        toolbar button. """
        TipText: str
        """ Get/set the tooltip text shown when the user hovers the toolbar button. """
        Visible: bool
        """ Get/set whether the toolbar button is visible or not. """
        Width: int
        """ Returns the width of the button, in pixels. """
        X: int
        """ Returns the X (horizontal) coordinate of the top-left corner of the button, in pixels. """
        Y: int
        """ Returns the Y (vertical) coordinate of the top-left corner of the button, in pixels. """

        def Execute(self) -> Optional[Any]:
            """
                Execute the command stored in the toolbar button.
                The command can be a UNO command or a Basic/Python script (expressed in the
                ``scripting framework_URI`` notation).
                    Returns
                        The output of the script or None
                """
            ...


# #############################################################################
# CreateScriptService signatures
# #############################################################################
@overload
def CreateScriptService(service: Literal['servicename', 'Servicename', 'Library.Servicename'], *args: Any
                        ) -> Optional[SERVICE]:
    """
        The modules and classes implemented in the ScriptForge libraries are invoked
        from user scripts as "Services". A generic constructor of those services has been designed for that purpose.
        
            Args
                ``service``: the name of the service as a string 'library.service' or one of its synonyms.

                The service names created with ``CreateScriptService()`` are:
                    - ``Array``, ``Basic``, ``Dictionary``, ``Exception``, ``FileSystem``, ``L10N``, ``Platform``, ``Region``, ``Session``, ``String``, ``Timer``, ``UI``
                    - ``Database``
                    - ``Dialog``, ``NewDialog``, ``DialogEvent``
                    - ``Document``, ``Base``, ``Calc``, ``Writer``
                    - ``PopupMenu``

                ``args``: the optional arguments to pass to the service constructor.
                        ``Dictionary``
                            (positional): a Python dictionary. Default = ``None``.
                        ``Timer``
                            ``start``: when ``True``, the timer starts immediately. Default = ``False``.
                        ``Database``
                            ``filename``: the name of the ``Base`` file containing the database. Must be expressed
                            using the ``SF_FileSystem.FileNaming`` notation.

                            ``registrationname``: the name of a registered database.
                            If ``filename`` is provided, this argument is ignored.

                            ``readonly``: determines if the database will be opened as readonly. Defaults to ``True``.

                            ``user``, ``password``: additional connection parameters.
                        ``Dialog``
                            ``container``:  ``"GlobalScope"`` for preinstalled libraries or a window name.
                            Defaults to the current document.

                            ``library``: the case-sensitive name of a library contained in the container.
                            Default value is ``"Standard"``.

                            ``dialogname``: a case-sensitive string designating the dialog.
                        ``NewDialog``
                            ``dialogname``: an identifier for the dialog to be built from scratch.

                            ``place``: the size and position expressed in "``APPFONT units``". Either:
                                - a tuple (X, Y, Width, Height).
                                - a ``com.sun.star.awt.Rectangle`` structure.
                        ``DialogEvent``
                            (positional): the event object passed by ``LibreOffice`` to the event-handling routine.
                        ``Document``, ``Base``, ``Calc``, ``Writer``
                            ``windowname``: see definitions at ``SF_UI`` class level.
                        ``PopupMenu``
                            ``event``: the mouse event object passed by ``LibreOffice`` to the event-handling routine.

                            ``x``, ``y``: the screen coordinates where the menu will be displayed.

                            ``submenuchar``: character or string that defines how menu items are nested.
                            The default character is ``">"``.
            Returns
                The service as a Python class instance.
        """
    ...
@overload
def CreateScriptService(service: Literal['array', 'Array', 'ScriptForge.Array']) -> ARRAY: ...
@overload
def CreateScriptService(service: Literal['basic', 'Basic', 'ScriptForge.Basic']) -> BASIC: ...
@overload
def CreateScriptService(service: Literal['dictionary', 'Dictionary', 'ScriptForge.Dictionary'],
                        dic: Optional[Dict] = None) -> Optional[DICTIONARY]: ...
@overload
def CreateScriptService(service: Literal['exception', 'Exception', 'ScriptForge.Exception']) -> EXCEPTION: ...
@overload
def CreateScriptService(service: Literal['filesystem', 'FileSystem', 'ScriptForge.FileSystem']) -> FILESYSTEM: ...
@overload
def CreateScriptService(service: Literal['l10n', 'L10N', 'ScriptForge.L10N'], foldername: str = '',
                        locale: str = '', encoding: str = '', locale2: str = '', encoding2: str = '') -> Optional[L10N]: ...
@overload
def CreateScriptService(service: Literal['platform', 'Platform', 'ScriptForge.Platform']) -> PLATFORM: ...
@overload
def CreateScriptService(service: Literal['region', 'Region', 'ScriptForge.Region']) -> Optional[REGION]: ...
@overload
def CreateScriptService(service: Literal['session', 'Session', 'ScriptForge.Session']) -> SESSION: ...
@overload
def CreateScriptService(service: Literal['string', 'String', 'ScriptForge.String']) -> STRING: ...
@overload
def CreateScriptService(service: Literal['timer', 'Timer', 'ScriptForge.Timer'], start: bool = False
                        ) -> Optional[TIMER]: ...
@overload
def CreateScriptService(service: Literal['ui', 'UI', 'ScriptForge.UI']) -> UI: ...
@overload
def CreateScriptService(service: Literal['database', 'Database', 'SFDatabases.Database'], filename: str = '',
                        registrationname: str = '', readonly: bool = True, user: str = '', password: str = ''
                        ) -> Optional[DATABASE]: ...
@overload
def CreateScriptService(service: Literal['dialog', 'Dialog', 'SFDialogs.Dialog'], container: str = '',
                        library: str = 'Standard', dialogname: str = '') -> Optional[DIALOG]: ...
@overload
def CreateScriptService(service: Literal['newdialog', 'NewDialog', 'SFDialogs.NewDialog'], dialogname: str,
                        place: Tuple[int, int, int, int]) -> Optional[DIALOG]: ...
@overload
def CreateScriptService(service: Literal['dialogevent', 'DialogEvent', 'SFDialogs.DialogEvent'],
                        event: Optional[UNO] = None) -> Optional[DIALOG, DIALOGCONTROL]: ...
@overload
def CreateScriptService(service: Literal['document', 'Document', 'SFDocuments.Document'],
                        windowname: Union[UNO, str] = '') -> Optional[DOCUMENT]: ...
@overload
def CreateScriptService(service: Literal['base', 'Base', 'SFDocuments.Base'], windowname: Union[UNO, str] = ''
                        ) -> Optional[BASE]: ...
@overload
def CreateScriptService(service: Literal['calc', 'Calc', 'SFDocuments.Calc'], windowname: Union[UNO, str] = ''
                        ) -> Optional[CALC]: ...
@overload
def CreateScriptService(service: Literal['writer', 'Writer', 'SFDocuments.Writer'], windowname: Union[UNO, str] = ''
                        ) -> Optional[WRITER]: ...
@overload
def CreateScriptService(service: Literal['popupmenu', 'PopupMenu', 'SFWidgets.PopupMenu'], event: Optional[UNO] = None,
                        x: int = 0, y: int = 0, submenuchar:str = '') -> Optional[POPUPMENU]: ...

createScriptService, createscriptservice = CreateScriptService, CreateScriptService
