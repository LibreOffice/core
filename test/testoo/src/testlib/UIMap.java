/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package testlib;
import java.io.File;

import org.openoffice.test.vcl.IDList;
import org.openoffice.test.vcl.widgets.VclApp;
import org.openoffice.test.vcl.widgets.VclButton;
import org.openoffice.test.vcl.widgets.VclComboBox;
import org.openoffice.test.vcl.widgets.VclControl;
import org.openoffice.test.vcl.widgets.VclDialog;
import org.openoffice.test.vcl.widgets.VclEditBox;
import org.openoffice.test.vcl.widgets.VclListBox;
import org.openoffice.test.vcl.widgets.VclMenuItem;
import org.openoffice.test.vcl.widgets.VclMessageBox;
import org.openoffice.test.vcl.widgets.VclStatusBar;
import org.openoffice.test.vcl.widgets.VclTabControl;
import org.openoffice.test.vcl.widgets.VclTabPage;
import org.openoffice.test.vcl.widgets.VclToolBox;
import org.openoffice.test.vcl.widgets.VclWindow;
import org.openoffice.test.vcl.widgets.VclField;
import org.openoffice.test.vcl.widgets.VclDockingWin;

/**
 * Define all UI controls in the class.
 * One control should be defined once and only once.
 *
 */
public class UIMap {
    private static IDList idList = new IDList(new File("./ids"));
    public static final VclMessageBox ActiveMsgBox = new VclMessageBox(idList.getId("UID_ACTIVE"));
    public static final VclMessageBox MsgBox_AdditionalRowsNotSaved = new VclMessageBox(idList.getId("UID_ACTIVE"), "Additional rows were not saved.");
    public static final VclTabControl ActiveTabControl = new VclTabControl(idList.getId("UID_ACTIVE"));

    public static VclEditBox editbox(String id) {
        return new VclEditBox(idList.getId(id));
    }


    public static VclControl control(String id) {
        return new VclControl(idList.getId(id));
    }

    public static VclWindow window(String id) {
        return new VclWindow(idList.getId(id));
    }

    public static VclDialog dialog(String id) {
        return new VclDialog(idList.getId(id));
    }

    public static VclTabPage tabpage(String id) {
        return new VclTabPage(idList.getId(id), ActiveTabControl);
    }

    public static VclListBox listbox(String id) {
        return new VclListBox(idList.getId(id));
    }

    public static VclComboBox combobox(String id) {
        return new VclComboBox(idList.getId(id));
    }

    public static VclToolBox toolbox(String id) {
        return new VclToolBox(idList.getId(id));
    }


    public static VclButton button(String id) {
        return new VclButton(idList.getId(id));
    }


    public static VclMessageBox msgbox(String text) {
        return new VclMessageBox(idList.getId("UID_ACTIVE"), text);
    }


    public static VclMenuItem menuItem(String text) {
        return new VclMenuItem(text);
    }

    public static VclStatusBar statusbar(String id) {
        return new VclStatusBar(idList.getId(id));
    }

    public static VclField field(String id){
        return new VclField(idList.getId(id));
    }

    public static VclDockingWin dockingwin(String id){
        return new VclDockingWin(idList.getId(id));
    }

    public static final VclApp app = new VclApp(null);
    public static final VclWindow writer = window("SW_HID_EDIT_WIN");
    public static final VclWindow startcenter = window("FWK_HID_BACKINGWINDOW");
    public static final VclWindow calc = window("SC_HID_SC_WIN_GRIDWIN");
    public static final VclWindow impress = window("SD_HID_SDDRAWVIEWSHELL");
    public static final VclDialog FilePicker = dialog("SVT_HID_EXPLORERDLG_FILE");
    public static final VclListBox FilePicker_FileType = listbox("fpicker:ListBox:DLG_SVT_EXPLORERFILE:LB_EXPLORERFILE_FILETYPE");
    public static final VclButton FilePicker_Open = button("fpicker:PushButton:DLG_SVT_EXPLORERFILE:BTN_EXPLORERFILE_OPEN");
    public static final VclComboBox FilePicker_Path = combobox("SVT_HID_FILEDLG_AUTOCOMPLETEBOX");
    public static final VclDialog FileSave = dialog("SVT_HID_FILESAVE_DIALOG");
    public static final VclListBox FileSave_FileType = listbox("SVT_HID_FILESAVE_FILETYPE");
    public static final VclButton FileSave_Save = button("SVT_HID_FILESAVE_DOSAVE");
    public static final VclComboBox FileSave_Path = combobox("SVT_HID_FILESAVE_FILEURL");
    public static final VclButton SC_InputBar_FunctionWizard = button("SC_HID_INSWIN_CALC");
    public static final VclButton SC_InputBar_Sum = button("SC_HID_INSWIN_SUMME");
    public static final VclButton SC_InputBar_Function = button("SC_HID_INSWIN_FUNC");
    public static final VclToolBox SC_InputBar = toolbox("SC_HID_SC_INPUTWIN");
    public static final VclComboBox SC_InputBar_Position = combobox("SC_HID_INSWIN_POS");
    public static final VclWindow SC_InputBar_Input = window("SC_HID_INSWIN_INPUT");
    public static final VclTabPage EffectsPage = tabpage("CUI_HID_SVXPAGE_CHAR_EFFECTS");
    public static final VclListBox EffectsPage_Color = listbox("cui:ListBox:RID_SVXPAGE_CHAR_EFFECTS:LB_FONTCOLOR");
    public static final VclDialog AlienFormatDlg = dialog("SFX2_HID_WARNING_ALIENFORMAT");
    public static final VclWindow draw = window("SD_HID_SDGRAPHICVIEWSHELL");
    public static final VclTabPage SortPage = tabpage("SC_HID_SCPAGE_SORT_FIELDS");
    public static final VclListBox SortPage_By1 = listbox("sc:ListBox:RID_SCPAGE_SORT_FIELDS:LB_SORT1");
    public static final VclButton SortPage_Ascending1 = button("sc:RadioButton:RID_SCPAGE_SORT_FIELDS:BTN_UP1");
    public static final VclButton SortPage_Descending1 = button("sc:RadioButton:RID_SCPAGE_SORT_FIELDS:BTN_DOWN1");
    public static final VclListBox SortPage_By2 = listbox("sc:ListBox:RID_SCPAGE_SORT_FIELDS:LB_SORT2");
    public static final VclButton SortPage_Ascending2 = button("sc:RadioButton:RID_SCPAGE_SORT_FIELDS:BTN_UP2");
    public static final VclButton SortPage_Descending2 = button("sc:RadioButton:RID_SCPAGE_SORT_FIELDS:BTN_DOWN2");
    public static final VclListBox SortPage_By3 = listbox("sc:ListBox:RID_SCPAGE_SORT_FIELDS:LB_SORT3");
    public static final VclButton SortPage_Ascending3 = button("sc:RadioButton:RID_SCPAGE_SORT_FIELDS:BTN_UP3");
    public static final VclButton SortPage_Descending3 = button("sc:RadioButton:RID_SCPAGE_SORT_FIELDS:BTN_DOWN3");
    public static final VclTabPage PDFGeneralPage = tabpage("HID_FILTER_PDF_OPTIONS");
    public static final VclButton StartCenter_CalcButton = button(".HelpId:StartCenter:CalcButton");
    public static final VclButton StartCenter_ImpressButton = button(".HelpId:StartCenter:ImpressButton");
    public static final VclButton StartCenter_WriterButton = button(".HelpId:StartCenter:WriterButton");
    public static final VclButton StartCenter_OpenButton = button(".HelpId:StartCenter:OpenButton");
    public static final VclButton StartCenter_DrawButton = button(".HelpId:StartCenter:DrawButton");
    public static final VclButton StartCenter_DBButton = button(".HelpId:StartCenter:DBButton");
    public static final VclButton StartCenter_MathButton = button(".HelpId:StartCenter:MathButton");
    public static final VclButton StartCenter_TemplateButton = button(".HelpId:StartCenter:TemplateButton");
    public static final VclButton StartCenter_TemplateRepository = button(".HelpId:StartCenter:TemplateRepository");
    public static final VclButton StartCenter_Extensions = button(".HelpId:StartCenter:Extensions");
    public static final VclButton StartCenter_Info = button(".HelpId:StartCenter:Info");
    public static final VclWindow SlideShow = window("SD_HID_SD_WIN_PRESENTATION");
    public static final VclDialog FindDlg = dialog(".uno:SearchDialog");
    public static final VclComboBox FindDlg_For = combobox("svx:ComboBox:RID_SVXDLG_SEARCH:ED_SEARCH");
    public static final VclComboBox FindDlg_ReplaceWith = combobox("svx:ComboBox:RID_SVXDLG_SEARCH:ED_REPLACE");
    public static final VclButton FindDlg_Find = button("svx:PushButton:RID_SVXDLG_SEARCH:BTN_SEARCH");
    public static final VclButton FindDlg_FindAll = button("svx:PushButton:RID_SVXDLG_SEARCH:BTN_SEARCH_ALL");
    public static final VclButton FindDlg_Replace = button("svx:PushButton:RID_SVXDLG_SEARCH:BTN_REPLACE");
    public static final VclButton FindDlg_ReplaceAll = button("svx:PushButton:RID_SVXDLG_SEARCH:BTN_REPLACE_ALL");
    public static final VclButton FindDlg_MatchCase = button("svx:CheckBox:RID_SVXDLG_SEARCH:CB_MATCH_CASE");
    public static final VclButton FindDlg_WholeWord = button("svx:CheckBox:RID_SVXDLG_SEARCH:CB_WHOLE_WORDS");
    public static final VclButton FindDlg_More = button("svx:MoreButton:RID_SVXDLG_SEARCH:BTN_MORE");
    public static final VclWindow chart = window("CHART2_HID_SCH_WIN_DOCUMENT");
    public static final VclDialog Chart_Wizard = dialog("CHART2_HID_SCH_CHART_AUTO_FORMAT");
    public static final VclDialog File_PrintDlg = dialog(".HelpID:vcl:PrintDialog:Dialog");
    public static final VclWindow ImpressOutline = window("SD_HID_SDOUTLINEVIEWSHELL");
    public static final VclWindow ImpressSlideSorter = window("SD_HID_SD_WIN_DOCUMENT");
    public static final VclWindow ImpressHandout = window(".uno:HandoutMode");
    public static final VclDialog InsertTable = dialog("cui:ModalDialog:RID_SVX_NEWTABLE_DLG");
    public static final VclToolBox Table_Toolbar = toolbox(".HelpId:tableobjectbar");
    public static final VclStatusBar StatusBar = statusbar("FWK_HID_STATUSBAR");
    public static final VclDialog writer_InsertTable = dialog(".uno:InsertTable");
    public static final VclWindow math_PreviewWindow = window("STARMATH_HID_SMA_WIN_DOCUMENT");
    public static final VclWindow math_EditWindow = window("STARMATH_HID_SMA_COMMAND_WIN_EDIT");
    public static final VclControl math_ElementsWindow = control("STARMATH_HID_SMA_OPERATOR_WIN");
    public static final VclEditBox SC_FunctionWizardDlg_MultiLineEdit = editbox("FORMULA_HID_FORMULA_FAP_FORMULA");
    public static final VclDialog SC_FunctionWizardDlg = dialog("FORMULA_HID_FORMULADLG_FORMULA");
    public static final VclListBox SC_FunctionWizardDlg_FunctionList = listbox("FORMULA_HID_FORMULA_LB_FUNCTION");
    public static final VclEditBox SC_FunctionWizardDlg_Edit1 = editbox("FORMULA_HID_FORMULA_FAP_EDIT1");
    public static final VclButton math_CatalogButton = button(".uno:SymbolCatalogue");
    public static final VclDialog math_SymbolsDlg = dialog("starmath:ModalDialog:RID_SYMBOLDIALOG");
    public static final VclListBox math_SymbolsDlgListbox = listbox("starmath:ListBox:RID_SYMBOLDIALOG:1");
    public static final VclButton math_SymbolsDlgEditButton = button("starmath:PushButton:RID_SYMBOLDIALOG:1");
    public static final VclButton math_SymbolsDlgInsertButton = button("starmath:PushButton:RID_SYMBOLDIALOG:2");
    public static final VclButton math_SymbolsDlgCloseButton = button("starmath:PushButton:RID_SYMBOLDIALOG:3");
    public static final VclDialog math_EditSymbolsDlg = dialog("starmath:ModalDialog:RID_SYMDEFINEDIALOG");
    public static final VclButton math_EditSymbolsDlgAdd = button("starmath:PushButton:RID_SYMDEFINEDIALOG:1");
    public static final VclButton math_EditSymbolsDlgModify = button("starmath:PushButton:RID_SYMDEFINEDIALOG:2");
    public static final VclButton math_EditSymbolsDlgDelete = button("starmath:PushButton:RID_SYMDEFINEDIALOG:3");
    public static final VclControl math_EditSymbolsDlgViewControl = control("STARMATH_HID_SMA_CONTROL_FONTCHAR_VIEW");
    public static final VclComboBox math_EditSymbolsDlgSymbol = combobox("starmath:ComboBox:RID_SYMDEFINEDIALOG:4");
    public static final VclListBox math_EditSymbolsDlgFont = listbox("starmath:ListBox:RID_SYMDEFINEDIALOG:1");
    public static final VclComboBox math_EditSymbolsDlgTypeface = combobox("starmath:ComboBox:RID_SYMDEFINEDIALOG:3");
    public static final VclDialog Wizards_AgendaDialog = dialog("WIZARDS_HID_AGWIZ");
    public static final VclButton Wizards_AgendaDialog_FinishButton = button("WIZARDS_HID_AGWIZ_CREATE");
    public static final VclTabPage MacroDialogsPage = tabpage("basctl:TabPage:RID_TP_DLGS");
    public static final VclDialog AboutDialog = dialog(".uno:About");
    public static final VclButton math_ElementsRelations = button("STARMATH_HID_SMA_RELATIONS_CAT");
    public static final VclButton math_ElementsRelationsNotEqual = button("STARMATH_HID_SMA_XNEQY");
    public static final VclButton math_ElementsUnaryBinary = button("STARMATH_HID_SMA_UNBINOPS_CAT");
    public static final VclButton math_ElementsUnaryBinaryPlus = button("STARMATH_HID_SMA_PLUSX");
    public static final VclControl MacroWarningOK = control("Ok");
    public static final VclListBox OptionsDlgList = listbox("CUI_HID_OFADLG_TREELISTBOX");
    public static final VclDialog OptionsDlg = dialog("CUI_HID_OFADLG_OPTIONS_TREE");
    public static final VclButton OptionsDlg_MacroSecurityButton = button("cui:PushButton:RID_SVXPAGE_INET_SECURITY:PB_SEC_MACROSEC");
    public static final VclControl MacroSecurityDlg = control("XMLSECURITY_HID_XMLSEC_TP_SECLEVEL");
    public static final VclButton MacroSecurityDlg_Low = button("xmlsecurity:RadioButton:RID_XMLSECTP_SECLEVEL:RB_LOW");
    public static final VclWindow Basic_EditWindow = window("BASCTL_HID_BASICIDE_EDITORWINDOW");
    public static final VclDialog writer_ConvertTableToTextDlg = dialog(".uno:ConvertTableText");
    public static final VclWindow writer_Navigator = window("SW_HID_NAVIGATION_PI");
    public static final VclListBox writer_NavigatorList = listbox("SW_HID_NAVIGATOR_TREELIST");
    public static final VclControl writer_FrameDlg = control("SW_HID_FRM_STD");
    public static final VclField SC_NumberOfColumnsRowsToInsert = field("1502452023");
    public static final VclDialog SC_InsertColumnsRowsdialog = dialog("26166");
    public static final VclButton SC_InsertColumnsRowsAfterSelectColumn = button("1502446395");
    public static final VclButton WizardNextButton = button("SVT_HID_WIZARD_NEXT");
    public static final VclTabPage ChartRangeChooseTabPage = tabpage("chart2:TabPage:TP_RANGECHOOSER");
    public static final VclButton WizardBackButton = button("SVT_HID_WIZARD_PREVIOUS");
    public static final VclTabPage ChartTypeChooseTabPage = tabpage("CHART2_HID_DIAGRAM_TYPE");
    public static final VclControl ChartWizardRoadMap = control("CHART2_HID_SCH_WIZARD_ROADMAP");
    public static final VclButton DataPilotTableToExistPlaceRadioButton = button("2569093658");
    public static final VclEditBox DataPilotTableToExistPlaceEditBox = editbox("2569095195");
    public static final VclDialog CreateDataPilotTableDialog = dialog("26151");
    public static final VclControl DataPilotFieldSelect = control("59030");
    public static final VclButton DataPilotAutomaticallyUpdateCheckBox = button("3643262003");
    public static final VclButton DataPilotTableUpdateButton = button("3643265586");
    public static final VclButton SD_DateAndTimeFooterOnSlide = button("553255938");
    public static final VclButton SD_FixedDateAndTimeFooterOnSlide = button("553255427");
    public static final VclEditBox SD_FixedDateAndTimeOnSlideInput = editbox("553256964");
    public static final VclButton SD_FooterTextOnSlide = button("553255946");
    public static final VclEditBox SD_FooterTextOnSlideInput = editbox("553256972");
    public static final VclButton SD_SlideNumAsFooterOnSlide = button("553255950");
    public static final VclButton SD_ApplyToAllButtonOnSlideFooter = button("553259537");
    public static final VclWindow SD_SlidesPanel = window("59787");
    public static final VclControl SD_SlidelistInNaviagtor = control("59793");
    public static final VclControl SD_HeaderAndFooterDialog = control("59901");
    public static final VclButton SD_ApplyButtonOnSlideFooter = button("553259538");
    public static final VclButton SD_InsertPageButtonOnToolbar = button(".uno:InsertPage");
    public static final VclControl DataPilotPaneRowArea = control("59036");
    public static final VclEditBox DataPilotFieldDisplayNameEditBox = editbox("971868166");
    public static final VclButton DataPilotFieldShowItemWithoutDataCheckBox = button("971867238");
    public static final VclTabPage DataPilotFieldOptionFieldTabPageDialog = tabpage("26550");
    public static final VclTabPage DataPilotFieldOptionSubTotalsTabPageDialog = tabpage("25760");
    public static final VclTabPage DataPilotFieldOptionSortTabPageDialog = tabpage("25763");
    public static final VclButton DataPilotFieldOptionSubTotalsManuallyRadioButton = button("958956137");
    public static final VclButton DataPilotFieldOptionSortDescendingRadioButton = button("958972620");
    public static final VclControl DataPiloPaneDataArea = control("59038");
    public static final VclControl DataPilotColumnArea = control("59034");
    public static final VclButton SD_HeaderTextOnNotes = button("553255958");
    public static final VclEditBox SD_HeaderTextOnNotesInput = editbox("553256984");
    public static final VclTabPage SD_HeaderAndFooterOnNotesTabPage = tabpage("59902");
    public static final VclButton SD_FooterNotShowOn1stSlide = button("553255952");
    public static final VclTabPage SortOptionsPage = tabpage("SC_HID_SCPAGE_SORT_OPTIONS");
    public static final VclButton SortOptionsPage_CaseSensitive = button("sc:CheckBox:RID_SCPAGE_SORT_OPTIONS:BTN_CASESENSITIVE");
    public static final VclButton SortOptionsPage_CopyResultTo = button("sc:CheckBox:RID_SCPAGE_SORT_OPTIONS:BTN_COPYRESULT");
    public static final VclEditBox SortOptionsPage_CopyResultToEdit = editbox("sc:Edit:RID_SCPAGE_SORT_OPTIONS:ED_OUTAREA");
    public static final VclDialog DefineNamesDlg = dialog(".uno:DefineName");
    public static final VclEditBox DefineNamesDlg_NameEdit = editbox("sc:Edit:RID_SCDLG_NAMES:ED_ASSIGN");
    public static final VclListBox SortOptionsPage_CopyResultToCellRange = listbox("sc:ListBox:RID_SCPAGE_SORT_OPTIONS:LB_OUTAREA");
    public static final VclWindow SpreadSheetCells = window("58818");
    public static final VclButton DataPilotFieldOptionSubTotalsNeverRadioButton = button("958956135");
    public static final VclButton DataPilotFieldOptionSortAscendingRadioButton = button("958972619");
    public static final VclControl DataPilotPanePageArea = control("59032");
    public static final VclDialog SaveInODFOrNot = dialog("33388");
    public static final VclButton OnlyDisplayTopNItemCheckBox = button("1495745540");
    public static final VclField NumberOfItemShowInTopNEditBox = field("1495750661");
    public static final VclDialog FieldTopNSettingDialog = dialog("59039");
    public static final VclButton SD_AutoUpdateTimeFooter = button("553255429");
    public static final VclListBox SD_AutoUpdateTimeFooterType = listbox("553258502");
    public static final VclButton DataPilotButton = button("59270");
    public static final VclDockingWin DataPilotPanel = dockingwin("3643260928");
    public static final VclListBox SCSubTotalsGroupByListBox = listbox("SC_HID_SC_SUBT_GROUP");
    public static final VclControl SCCalcSubTotalForColumns = control("SC_HID_SC_SUBT_COLS");
    public static final VclTabPage SCSubTotalsGroup1Dialog = tabpage("SC_HID_SCPAGE_SUBT_GROUP1");
    public static final VclButton SCSubtotalsInsertPageBreakCheckBox = button("sc:CheckBox:RID_SCPAGE_SUBT_OPTIONS:BTN_PAGEBREAK");
    public static final VclButton SCSubtotalsCaseSensitiveCheckBox = button("sc:CheckBox:RID_SCPAGE_SUBT_OPTIONS:BTN_CASE");
    public static final VclButton SCSubtotalsPreSortToGroupCheckBox = button("sc:CheckBox:RID_SCPAGE_SUBT_OPTIONS:BTN_SORT");
    public static final VclButton SCSubtotalSortAscendingRadioButton = button("sc:RadioButton:RID_SCPAGE_SUBT_OPTIONS:BTN_ASCENDING");
    public static final VclButton SCSubtotalSortDescendingRadioButton = button("sc:RadioButton:RID_SCPAGE_SUBT_OPTIONS:BTN_DESCENDING");
    public static final VclButton SCSubtotalsIncludeFormatsCheckBox = button("sc:CheckBox:RID_SCPAGE_SUBT_OPTIONS:BTN_FORMATS");
    public static final VclButton SCSubtotalsCustomSortOrderCheckBox = button("sc:CheckBox:RID_SCPAGE_SUBT_OPTIONS:BTN_USERDEF");
    public static final VclListBox SCSubtotalsCustomSortListBox = listbox("sc:ListBox:RID_SCPAGE_SUBT_OPTIONS:LB_USERDEF");
    public static final VclListBox SC_ValidityCriteriaAllowList = listbox("548277762");
    public static final VclTabPage SC_ValidityCriteriaTabpage = tabpage("548274176");
    public static final VclButton SC_ValidityAllowBlankCells = button("548275203");
    public static final VclButton SC_ValidityShowSelectionList = button("548275227");
    public static final VclButton SC_ValiditySortEntriesAscending = button("548275228");
    public static final VclButton SC_ValiditySourcePicker = button("548286040");
    public static final VclTabPage SC_ValidityErrorAlertTabPage = tabpage("548306944");
    public static final VclButton SC_ValidityShowErrorMessage = button("548308501");
    public static final VclListBox SC_ValidityErrorAlertActionList = listbox("548310552");
    public static final VclEditBox SC_ValidityErrorMessageTitle = editbox("548309005");
    public static final VclEditBox SC_ValidityErrorMessage = editbox("548309529");
    public static final VclButton SC_ValidityErrorBrowseButton = button("548311578");
    public static final VclTabPage SCSubTotalsOptionsTabPage = tabpage("SC_HID_SCPAGE_SUBT_OPTIONS");
    public static final VclButton SCDeleteCellsDeleteRowsRadioButton = button("sc:RadioButton:RID_SCDLG_DELCELL:BTN_DELROWS");
    public static final VclDialog SCDeleteCellsDialog = dialog(".uno:DeleteCell");
    public static final VclTabPage SCSubTotalsGroup2Dialog = tabpage("SC_HID_SCPAGE_SUBT_GROUP2");
    public static final VclTabPage SCSubTotalsGroup3Dialog = tabpage("SC_HID_SCPAGE_SUBT_GROUP3");
    public static final VclListBox SCCalcSubTotolsFuncionList = listbox("SC_HID_SC_SUBT_FUNC");
    public static final VclEditBox SC_ValiditySourceInput = editbox("548276231");
    public static final VclWindow SC_CellInput = window("58774");
    public static final VclListBox SC_ValidityDecimalCompareOperator = listbox("548277765");
    public static final VclEditBox SC_ValidityMaxValueInput = editbox("548276233");
    public static final VclEditBox SC_ValidityEntries = editbox("548276765");
    public static final VclTabPage SC_ValidityInputHelpTabPage = tabpage("548290560");
    public static final VclButton SC_ValidityInputHelpCheckbox = button("548292107");
    public static final VclEditBox SC_ValidityInputHelpTitle = editbox("548292621");
    public static final VclEditBox SC_ValidityHelpMessage = editbox("548293135");
    public static final VclButton OptionsDlg_Copy = button("sc:PushButton:RID_SCPAGE_USERLISTS:BTN_COPY");
    public static final VclButton SortOptionsPage_RangeContainsColumnLabels = button("sc:CheckBox:RID_SCPAGE_SORT_OPTIONS:BTN_LABEL");
    public static final VclButton SortOptionsPage_CustomSortOrder = button("sc:CheckBox:RID_SCPAGE_SORT_OPTIONS:BTN_SORT_USER");
    public static final VclButton SortOptionsPage_IncludeFormats = button("sc:CheckBox:RID_SCPAGE_SORT_OPTIONS:BTN_FORMATS");
    public static final VclListBox SortOptionsPage_CustomSortOrderList = listbox("sc:ListBox:RID_SCPAGE_SORT_OPTIONS:LB_SORT_USER");
    public static final VclButton SortOptionsPage_TopToBottom = button("sc:RadioButton:RID_SCPAGE_SORT_OPTIONS:BTN_TOP_DOWN");
    public static final VclButton SortOptionsPage_LeftToRight = button("sc:RadioButton:RID_SCPAGE_SORT_OPTIONS:BTN_LEFT_RIGHT");
    public static final VclButton OptionsDlg_New = button("sc:PushButton:RID_SCPAGE_USERLISTS:BTN_NEW");
    public static final VclButton OptionsDlg_Add = button("sc:PushButton:RID_SCPAGE_USERLISTS:BTN_ADD");
    public static final VclTabPage FormatCellsDlg_NumbersPage = tabpage("CUI_HID_NUMBERFORMAT");
    public static final VclListBox FormatCellsDlg_NumbersPageCategory = listbox("cui:ListBox:RID_SVXPAGE_NUMBERFORMAT:LB_CATEGORY");
    public static final VclListBox FormatCellsDlg_NumbersPageCurrencyFormat = listbox("cui:ListBox:RID_SVXPAGE_NUMBERFORMAT:LB_CURRENCY");
    public static final VclTabPage DataPilotFieldOptionFiledSubtotalsPage = tabpage("25762");
    public static final VclControl SC_DataPilotOptionSubtotalFunctionList = control("59029");
    public static final VclDialog PresentationWizard = dialog("SD_HID_SD_AUTOPILOT_PAGE1");
    public static final VclDialog FillSeriesDlg = dialog(".uno:FillSeries");
    public static final VclDialog SortWarningDlg = dialog("sc:ModalDialog:RID_SCDLG_SORT_WARNING");
    public static final VclButton SortWarningDlg_Extend = button("sc:PushButton:RID_SCDLG_SORT_WARNING:BTN_EXTSORT");
    public static final VclButton SortWarningDlg_Current = button("sc:PushButton:RID_SCDLG_SORT_WARNING:BTN_CURSORT");
    // Spreadsheet: "Sheet" dialog
    public static final VclDialog SCInsertSheetDlg = dialog("sc:ModalDialog:RID_SCDLG_INSERT_TABLE");
    public static final VclButton SCBeforeCurrentSheet = button("sc:RadioButton:RID_SCDLG_INSERT_TABLE:RB_BEFORE");
    public static final VclButton SCAfterCurrentSheet = button("sc:RadioButton:RID_SCDLG_INSERT_TABLE:RB_BEHIND");
    public static final VclButton SCNewSheet = button("sc:RadioButton:RID_SCDLG_INSERT_TABLE:RB_NEW");
    public static final VclButton SCNewSheetFromFile = button("sc:RadioButton:RID_SCDLG_INSERT_TABLE:RB_FROMFILE");
    public static final VclEditBox SCNewSheetName = editbox("sc:Edit:RID_SCDLG_INSERT_TABLE:ED_TABNAME");
    public static final VclField SCSheetNumber = field("sc:NumericField:RID_SCDLG_INSERT_TABLE:NF_COUNT");
    public static final VclListBox SCFromFileSheetList = listbox("sc:MultiListBox:RID_SCDLG_INSERT_TABLE:LB_TABLES");
    public static final VclButton SCFromFileBrowse = button("sc:PushButton:RID_SCDLG_INSERT_TABLE:BTN_BROWSE");
    public static final VclButton SCFromFileLink = button("sc:CheckBox:RID_SCDLG_INSERT_TABLE:CB_LINK");
    // Spreadsheet: "Select Sheets" dialog
    public static final VclDialog SCSelectSheetsDlg = dialog(".uno:SelectTables");
    public static final VclListBox SCSheetsList = listbox("SC_HID_SELECTTABLES");



}
