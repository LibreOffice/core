rem *************************************************************
rem  
rem  Licensed to the Apache Software Foundation (ASF) under one
rem  or more contributor license agreements.  See the NOTICE file
rem  distributed with this work for additional information
rem  regarding copyright ownership.  The ASF licenses this file
rem  to you under the Apache License, Version 2.0 (the
rem  "License"); you may not use this file except in compliance
rem  with the License.  You may obtain a copy of the License at
rem  
rem    http://www.apache.org/licenses/LICENSE-2.0
rem  
rem  Unless required by applicable law or agreed to in writing,
rem  software distributed under the License is distributed on an
rem  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
rem  KIND, either express or implied.  See the License for the
rem  specific language governing permissions and limitations
rem  under the License.
rem  
rem *************************************************************
Attribute VB_Name = "results_res"
Public RID_RESXLS_COST_Action_Settings As String
Public RID_RESXLS_COST_Add_custom_properties_to_document_Properties_Summary_Comments_field As String
Public RID_RESXLS_COST_Additional_Weighting_Factors As String
Public RID_RESXLS_COST_All_sheets__256_will_need_to_be_moved_to_a_new_workbook As String
Public RID_RESXLS_COST_Appearance As String
Public RID_RESXLS_COST_Application As String
Public RID_RESXLS_COST_Border_Styles As String
Public RID_RESXLS_COST_Cell_Attributes As String
Public RID_RESXLS_COST_CELL_SPAN_WORKAROUND As String
Public RID_RESXLS_COST_Cell_Spanning_Page As String
Public RID_RESXLS_COST_ChangesAndReviewing As String
Public RID_RESXLS_COST_Charts_And_Tables As String
Public RID_RESXLS_COST_Comment As String
Public RID_RESXLS_COST_Content_And_Document_Properties As String
Public RID_RESXLS_COST_ContentAndDocumentProperties As String
Public RID_RESXLS_COST_Controls As String
Public RID_RESXLS_COST_Cost As String
Public RID_RESXLS_COST_Cost__In_simple_cases_cut_and_paste_to_new_sheet_3___5_min As String
Public RID_RESXLS_COST_COST_0 As String
Public RID_RESXLS_COST_COST_EMBED_CHART As String
Public RID_RESXLS_COST_COST_ERROR_TYPE As String
Public RID_RESXLS_COST_COST_INFO As String
Public RID_RESXLS_COST_COST_INVAL_PASS As String
Public RID_RESXLS_COST_COST_INVALID_WORKSHEET_NAME As String
Public RID_RESXLS_COST_Cost_of_4_min_per_line__takes_no_account_of_complexity_of_code__number_of_comment_lines_and_so_on As String
Public RID_RESXLS_COST_Cost_per_Issue_or_Factor__min As String
Public RID_RESXLS_COST_COST_PIVOT As String
Public RID_RESXLS_COST_COST_SHEET_CHART As String
Public RID_RESXLS_COST_Cost_variations_occur_because_of_the_use_of_macros_with_controls As String
Public RID_RESXLS_COST_COST_VBA_CONT_TYPE As String
Public RID_RESXLS_COST_COST_VBA_CONTROLS As String
Public RID_RESXLS_COST_COST_VBA_FORMS As String
Public RID_RESXLS_COST_COST_VBA_LOC As String
Public RID_RESXLS_COST_COST_VBA_PROPS As String
Public RID_RESXLS_COST_Costs__min As String
Public RID_RESXLS_COST_Custom_Bullet_List As String
Public RID_RESXLS_COST_Document_Custom_Properties As String
Public RID_RESXLS_COST_Document_Parts_Protection As String
Public RID_RESXLS_COST_Embedded_Chart As String
Public RID_RESXLS_COST_embedded_objects As String
Public RID_RESXLS_COST_Embedded_OLE_Object As String
Public RID_RESXLS_COST_Excel As String
Public RID_RESXLS_COST_Excel_Totals As String
Public RID_RESXLS_COST_External As String
Public RID_RESXLS_COST_External_References_In_Macro___Module__Class_or_UserForm As String
Public RID_RESXLS_COST_Factor_Count As String
Public RID_RESXLS_COST_Fields As String
Public RID_RESXLS_COST_Filters As String
Public RID_RESXLS_COST_Fixed_SO7 As String
Public RID_RESXLS_COST_Form_Field As String
Public RID_RESXLS_COST_Format As String
Public RID_RESXLS_COST_Functions As String
Public RID_RESXLS_COST_HEADERFOOTER_WORKAROUND As String
Public RID_RESXLS_COST_Hours_Per_MD As String
Public RID_RESXLS_COST_Indexes_And_References As String
Public RID_RESXLS_COST_Invalid_Password_Entered As String
Public RID_RESXLS_COST_Invalid_Worksheet_Name As String
Public RID_RESXLS_COST_Issue__18_of_the_24_table_border_styles_do_not_display_correctly As String
Public RID_RESXLS_COST_Issue__256_sheet_limit_exceeded As String
Public RID_RESXLS_COST_Issue__32000_row_limit_exceeded As String
Public RID_RESXLS_COST_Issue__A_worksheet_in_the_workbook_has_a_name_with_invalid_characters As String
Public RID_RESXLS_COST_Issue__Calls_to_windows_API_functions_are_windows_specific As String
Public RID_RESXLS_COST_Issue__Chart_support_is_limited_in_SO7 As String
Public RID_RESXLS_COST_Issue__Comments_are_lost_on_migration_to_SO7 As String
Public RID_RESXLS_COST_Issue__Custom_Properties_are_lost_on_migration As String
Public RID_RESXLS_COST_Issue__Document_is_password_protected_and_cannot_be_opened_by_SO As String
Public RID_RESXLS_COST_Issue__External_functions_not_supported_by_SO7 As String
Public RID_RESXLS_COST_Issue__External_references_are_windows_specific As String
Public RID_RESXLS_COST_Issue__Fields_are_imported_but_need_to_have_the_link_to_datasource_reset As String
Public RID_RESXLS_COST_Issue__Graphics___frames_in_header_footer_tend_to_be_misplaced As String
Public RID_RESXLS_COST_Issue__Incorrect_password_entered As String
Public RID_RESXLS_COST_Issue__Information_relating_to_the_data_source_is_lost_in_migration As String
Public RID_RESXLS_COST_Issue__Invalid_password_entered_Rerun_analysis As String
Public RID_RESXLS_COST_Issue__Lost_during_migration_No_StarOffice_equivalent As String
Public RID_RESXLS_COST_Issue__Macros_from_old_workbook_versions_do_not_import_into_SO7 As String
Public RID_RESXLS_COST_Issue__Modules_will_need_porting_to_StarOffice_Basic As String
Public RID_RESXLS_COST_Issue__Most_migrate_ok_but_do_not_roundtrip As String
Public RID_RESXLS_COST_Issue__Movies_are_not_supported_by_SO7 As String
Public RID_RESXLS_COST_Issue__Need_to_set_these_to_be_included_in_TOC_in_SO7 As String
Public RID_RESXLS_COST_Issue__Password_protection_of_comments__tracked_changes_and_forms_is_lost As String
Public RID_RESXLS_COST_Issue__Pivot_charts_are_not_supported_by_SO7 As String
Public RID_RESXLS_COST_Issue__Presents_a_cross_platform_interoperability_issue As String
Public RID_RESXLS_COST_Issue__Print_wide_pages_not_supported As String
Public RID_RESXLS_COST_Issue__Shading_can_be_applied_to_fields_and_to_controls_separately_in_Word As String
Public RID_RESXLS_COST_Issue__Table_imported_as_text_as_no_StarOffice_equivalent As String
Public RID_RESXLS_COST_Issue__Tables_nested_in_tables_are_not_supported_by_SO7 As String
Public RID_RESXLS_COST_Issue__Tabs_are_incorrect_after_migration As String
Public RID_RESXLS_COST_Issue__Text_form_fields_roundtrip_as_Fill_In_field As String
Public RID_RESXLS_COST_Issue__Unsupported_formatting_set As String
Public RID_RESXLS_COST_Issue__Unsupported_function_type As String
Public RID_RESXLS_COST_Issue__Userform_controls_will_need_porting_to_StarOffice_Basic As String
Public RID_RESXLS_COST_Issue__Userforms_controls_will_need_porting_to_StarOffice_Basic As String
Public RID_RESXLS_COST_Issue__Userforms_will_need_porting_to_StarOffice_Basic As String
Public RID_RESXLS_COST_Issue__VBProject_is_password_protected_Remove_password__rerun_analysis As String
Public RID_RESXLS_COST_Issue__Will_need_refreshing_in_SO7_External_datasources_and_functions_are_not_supported As String
Public RID_RESXLS_COST_Issue__Workbook_protection_is_not_supported As String
Public RID_RESXLS_COST_Issue__Writer_cannot_correctly_display_cells_spanning_more_than_1_page As String
Public RID_RESXLS_COST_Issue_Category As String
Public RID_RESXLS_COST_Issue_Count As String
Public RID_RESXLS_COST_Issue_Type As String
Public RID_RESXLS_COST_Linked_OLE_Object As String
Public RID_RESXLS_COST_Mail_Merge_Datasource As String
Public RID_RESXLS_COST_Mail_Merge_Field As String
Public RID_RESXLS_COST_Major_Issue As String
Public RID_RESXLS_COST_MAX_SHEETS_WORKAROUND As String
Public RID_RESXLS_COST_Maximum_Rows_Exceeded As String
Public RID_RESXLS_COST_Maximum_Sheets_Exceeded As String
Public RID_RESXLS_COST_Migration_Issues_Costs As String
Public RID_RESXLS_COST_Movie As String
Public RID_RESXLS_COST_NESTED_TABLE_WORKAROUND As String
Public RID_RESXLS_COST_Nested_Tables As String
Public RID_RESXLS_COST_Not_Planned As String
Public RID_RESXLS_COST_Notes_And_Handouts As String
Public RID_RESXLS_COST_Number_Of_External_References As String
Public RID_RESXLS_COST_Number_Of_Lines As String
Public RID_RESXLS_COST_Number_of_Lines_in_Unique_Modules As String
Public RID_RESXLS_COST_Number_of_Modules As String
Public RID_RESXLS_COST_Number_of_Unique_Modules As String
Public RID_RESXLS_COST_Numbering_Reference As String
Public RID_RESXLS_COST_Numbering_Reference_Comment As String
Public RID_RESXLS_COST_Object_In_Header_Footer As String
Public RID_RESXLS_COST_Objects_And_Graphics As String
Public RID_RESXLS_COST_Objects_Graphics_And_Frames As String
Public RID_RESXLS_COST_Objects_Graphics_And_Textboxes As String
Public RID_RESXLS_COST_Old_Workbook_Version As String
Public RID_RESXLS_COST_OLE_Control As String
Public RID_RESXLS_COST_OLECONTR_COST As String
Public RID_RESXLS_COST_Page_Setup As String
Public RID_RESXLS_COST_Password_Protected As String
Public RID_RESXLS_COST_Password_Protection As String
Public RID_RESXLS_COST_Pivot As String
Public RID_RESXLS_COST_Planned_SO8 As String
Public RID_RESXLS_COST_Portability As String
Public RID_RESXLS_COST_PowerPoint As String
Public RID_RESXLS_COST_PowerPoint_Totals As String
Public RID_RESXLS_COST_Prepared_Issue_Count As String
Public RID_RESXLS_COST_Prepared_Savings__min As String
Public RID_RESXLS_COST_Properties___Module__Class_or_UserForm As String
Public RID_RESXLS_COST_Remove_password_to_open As String
Public RID_RESXLS_COST_Remove_the_invalid_characters_from_the_worksheet_name As String
Public RID_RESXLS_COST_Rerun_analysis_with_correct_password As String
Public RID_RESXLS_COST_Reset_link_to_Data_Base_having_setup_a_Datasource_in_SO7 As String
Public RID_RESXLS_COST_set_cost_factor_if_the_macro_has_to_be_ported_off_Windows As String
Public RID_RESXLS_COST_Save_workbook_as_a_new_workbook_version As String
Public RID_RESXLS_COST_Setup_Datasource_in_SO7_using_Tools__Datasource As String
Public RID_RESXLS_COST_Sheet_Chart As String
Public RID_RESXLS_COST_Status As String
Public RID_RESXLS_COST_Table_Of_Authorities As String
Public RID_RESXLS_COST_Table_Of_Authorities_Field As String
Public RID_RESXLS_COST_Table_Of_Contents As String
Public RID_RESXLS_COST_Tables As String
Public RID_RESXLS_COST_TOC_ISSUE_WORKAROUND As String
Public RID_RESXLS_COST_Total_Cost___MD As String
Public RID_RESXLS_COST_User_Error As String
Public RID_RESXLS_COST_User_Forms_Control_Count As String
Public RID_RESXLS_COST_User_Forms_Control_Type_Count As String
Public RID_RESXLS_COST_User_Forms_Count As String
Public RID_RESXLS_COST_VBA_Macros As String
Public RID_RESXLS_COST_Word As String
Public RID_RESXLS_COST_Word_Totals As String
Public RID_RESXLS_COST_Work_Around As String
Public RID_RESXLS_COST_Workaround_Remove_protection_before_importing As String
Public RID_RESXLS_COST_WorkbookProtection As String
Public RID_RESXLS_DP_Accessed As String
Public RID_RESXLS_DP_All_Analysed_Documents___Properties As String
Public RID_RESXLS_DP_Application As String
Public RID_RESXLS_DP_Based_on_Template As String
Public RID_RESXLS_DP_Costs As String
Public RID_RESXLS_DP_Created As String
Public RID_RESXLS_DP_Document_Details As String
Public RID_RESXLS_DP_Document_Issue_Costs As String
Public RID_RESXLS_DP_Document_Migration_Issues As String
Public RID_RESXLS_DP_Document_Name As String
Public RID_RESXLS_DP_Document_Name_and_Path As String
Public RID_RESXLS_DP_Issues_Complex_count As String
Public RID_RESXLS_DP_Last_Saved_By As String
Public RID_RESXLS_DP_Lines_of_Macro_Code As String
Public RID_RESXLS_DP_Macro_Issues As String
Public RID_RESXLS_DP_Macro_Migration_Costs As String
Public RID_RESXLS_DP_Macro_Migration_Issues As String
Public RID_RESXLS_DP_Migration_Issues As String
Public RID_RESXLS_DP_Minor_Issues As String
Public RID_RESXLS_DP_Modified As String
Public RID_RESXLS_DP_Pages_Sheets_Slides As String
Public RID_RESXLS_DP_Prepareable_Issues_Costs As String
Public RID_RESXLS_DP_Prepared_Issues As String
Public RID_RESXLS_DP_Printed As String
Public RID_RESXLS_DP_Revision As String
Public RID_RESXLS_DP_User_Form_Count As String
Public RID_RESXLS_ID_All_Documents_with_Issues___Issue_Details As String
Public RID_RESXLS_ID_Application As String
Public RID_RESXLS_ID_Column__Left As String
Public RID_RESXLS_ID_Document_Name As String
Public RID_RESXLS_ID_Document_Name_and_Path As String
Public RID_RESXLS_ID_Issue_Category As String
Public RID_RESXLS_ID_Issue_Details As String
Public RID_RESXLS_ID_Issue_Type As String
Public RID_RESXLS_ID_Line__Row__Top As String
Public RID_RESXLS_ID_Location As String
Public RID_RESXLS_ID_Location_Type As String
Public RID_RESXLS_OV_3_to_6_months As String
Public RID_RESXLS_OV_6_to_12_months As String
Public RID_RESXLS_OV_Complex As String
Public RID_RESXLS_OV_Cost_estimates_for_migration As String
Public RID_RESXLS_OV_Costs As String
Public RID_RESXLS_OV_Docs_with_DocumentMigrationIssues As String
Public RID_RESXLS_OV_Document_Migration_Costs As String
Public RID_RESXLS_OV_Document_Modification_Dates As String
Public RID_RESXLS_OV_Document_Type As String
Public RID_RESXLS_OV_Documents_with_Macro_Migration_Issues As String
Public RID_RESXLS_OV_Excel As String
Public RID_RESXLS_OV_Excel_Costs As String
Public RID_RESXLS_OV_Excel_Spreadsheet As String
Public RID_RESXLS_OV_Excel_Template As String
Public RID_RESXLS_OV_GREATER_THAN_1_year As String
Public RID_RESXLS_OV_Last_Modified As String
Public RID_RESXLS_OV_LESS_3_months As String
Public RID_RESXLS_OV_LESS_THAN3MONTHS As String
Public RID_RESXLS_OV_Macro_Migration_Costs As String
Public RID_RESXLS_OV_Medium As String
Public RID_RESXLS_OV_Minor As String
Public RID_RESXLS_OV_None As String
Public RID_RESXLS_OV_Number As String
Public RID_RESXLS_OV_Number_of_Documents_Analyzed As String
Public RID_RESXLS_OV_NumberOfDocs As String
Public RID_RESXLS_OV_Potential_savings_macro_conversion As String
Public RID_RESXLS_OV_Potential_savings_prepared_docs As String
Public RID_RESXLS_OV_PowerPoint As String
Public RID_RESXLS_OV_PowerPoint_Costs As String
Public RID_RESXLS_OV_PowerPoint_Document As String
Public RID_RESXLS_OV_PowerPoint_Template As String
Public RID_RESXLS_OV_Simple As String
Public RID_RESXLS_OV_Total As String
Public RID_RESXLS_OV_Totals As String
Public RID_RESXLS_OV_Word As String
Public RID_RESXLS_OV_Word_Costs As String
Public RID_RESXLS_OV_Word_Document As String
Public RID_RESXLS_OV_Word_Template As String
Public RID_RESXLS_PP_Action_Settings As String
Public RID_RESXLS_PP_Application As String
Public RID_RESXLS_PP_Content___Document_Properties As String
Public RID_RESXLS_PP_Document_Name_and_Path As String
Public RID_RESXLS_PP_Fields As String
Public RID_RESXLS_PP_Format As String
Public RID_RESXLS_PP_Notes___Handouts As String
Public RID_RESXLS_PP_Objects__Graphics___Textboxes As String
Public RID_RESXLS_PP_Portability As String
Public RID_RESXLS_PP_PowerPoint_Documents_with_Issues___Issue_Summary As String
Public RID_RESXLS_PP_PowerPoint_Presentation_Name As String
Public RID_RESXLS_PP_VBA_Macros As String
Public RID_RESXLS_RD_All_Documents_with_Macros___Reference_Details As String
Public RID_RESXLS_RD_Application As String
Public RID_RESXLS_RD_Description As String
Public RID_RESXLS_RD_Document_Name As String
Public RID_RESXLS_RD_Document_Name_and_Path As String
Public RID_RESXLS_RD_Location As String
Public RID_RESXLS_RD_Reference As String
Public RID_RESXLS_RD_Reference_Details As String
Public RID_RESXLS_WI_Application As String
Public RID_RESXLS_WI_Changes_and_Reviewing As String
Public RID_RESXLS_WI_Content___Document_Properties As String
Public RID_RESXLS_WI_Controls As String
Public RID_RESXLS_WI_Document_Name_and_Path As String
Public RID_RESXLS_WI_Fields As String
Public RID_RESXLS_WI_Format As String
Public RID_RESXLS_WI_Index_and_References As String
Public RID_RESXLS_WI_Objects__Graphics____Frames As String
Public RID_RESXLS_WI_Portability As String
Public RID_RESXLS_WI_Tables As String
Public RID_RESXLS_WI_VBA_Macros As String
Public RID_RESXLS_WI_Word_Document_Name As String
Public RID_RESXLS_WI_Word_Documents_with_Issues___Issue_Summary As String
Public RID_RESXLS_XL_Application As String
Public RID_RESXLS_XL_Changes___Reviewing As String
Public RID_RESXLS_XL_Charts___Tables As String
Public RID_RESXLS_XL_Excel_Documents_with_Issues___Issue_Summary As String
Public RID_RESXLS_XL_Excel_Spreadsheet_Name As String
Public RID_RESXLS_XL_Format As String
Public RID_RESXLS_XL_Functions As String
Public RID_RESXLS_XL_Objects___Graphics As String

Public RID_RESXLS_COST_DB_Query As String
Public RID_RESXLS_COST_DB_Query_Comment As String
Public RID_RESXLS_COST_LineStyle As String
Public RID_RESXLS_COST_LineStyle_Comment As String
Public RID_RESXLS_COST_Numbering As String
Public RID_RESXLS_COST_Numbering_Comment As String
Public RID_RESXLS_COST_Template As String
Public RID_RESXLS_COST_Template_Comment As String
Public RID_RESXLS_COST_DATEDIF_Note As String
Public RID_RESXLS_COST_Tabstop As String
Public RID_RESXLS_COST_Tabstop_Comment As String
Public RID_RESXLS_COST_Transparent As String
Public RID_RESXLS_COST_Transparent_Comment As String
Public RID_RESXLS_COST_GradientStyle As String
Public RID_RESXLS_COST_GradientStyle_Comment As String
Public RID_RESXLS_COST_GradientStyle_Note As String
Public RID_RESXLS_COST_Hyperlink As String
Public RID_RESXLS_COST_Hyperlink_Comment As String
Public RID_RESXLS_COST_HyperlinkSplit As String
Public RID_RESXLS_COST_HyperlinkSplit_Comment As String
Public RID_RESXLS_COST_Background As String
Public RID_RESXLS_COST_Background_Comment As String

Public Sub LoadResultsStrings(sdm As StringDataManager)
    On Error Resume Next

    sdm.InitString RID_RESXLS_COST_Action_Settings, "RID_RESXLS_COST_Action_Settings"
    sdm.InitString RID_RESXLS_COST_Add_custom_properties_to_document_Properties_Summary_Comments_field, "RID_RESXLS_COST_Add_custom_properties_to_document_Properties_Summary_Comments_field"
    sdm.InitString RID_RESXLS_COST_Additional_Weighting_Factors, "RID_RESXLS_COST_Additional_Weighting_Factors"
    sdm.InitString RID_RESXLS_COST_All_sheets__256_will_need_to_be_moved_to_a_new_workbook, "RID_RESXLS_COST_All_sheets__256_will_need_to_be_moved_to_a_new_workbook"
    sdm.InitString RID_RESXLS_COST_Appearance, "RID_RESXLS_COST_Appearance"
    sdm.InitString RID_RESXLS_COST_Application, "RID_RESXLS_COST_Application"
    sdm.InitString RID_RESXLS_COST_Border_Styles, "RID_RESXLS_COST_Border_Styles"
    sdm.InitString RID_RESXLS_COST_Cell_Attributes, "RID_RESXLS_COST_Cell_Attributes"
    sdm.InitString RID_RESXLS_COST_CELL_SPAN_WORKAROUND, "RID_RESXLS_COST_CELL_SPAN_WORKAROUND"
    sdm.InitString RID_RESXLS_COST_Cell_Spanning_Page, "RID_RESXLS_COST_Cell_Spanning_Page"
    sdm.InitString RID_RESXLS_COST_ChangesAndReviewing, "RID_RESXLS_COST_ChangesAndReviewing"
    sdm.InitString RID_RESXLS_COST_Charts_And_Tables, "RID_RESXLS_COST_Charts_And_Tables"
    sdm.InitString RID_RESXLS_COST_Comment, "RID_STR_WORD_SUBISSUE_COMMENT"
    sdm.InitString RID_RESXLS_COST_Content_And_Document_Properties, "RID_RESXLS_COST_Content_And_Document_Properties"
    sdm.InitString RID_RESXLS_COST_ContentAndDocumentProperties, "RID_RESXLS_COST_ContentAndDocumentProperties"
    sdm.InitString RID_RESXLS_COST_Controls, "RID_RESXLS_COST_Controls"
    sdm.InitString RID_RESXLS_COST_Cost, "RID_RESXLS_COST_Cost"
    sdm.InitString RID_RESXLS_COST_Cost__In_simple_cases_cut_and_paste_to_new_sheet_3___5_min, "RID_RESXLS_COST_Cost__In_simple_cases_cut_and_paste_to_new_sheet_3___5_min"
    sdm.InitString RID_RESXLS_COST_COST_0, "RID_RESXLS_COST_COST_0"
    sdm.InitString RID_RESXLS_COST_COST_EMBED_CHART, "RID_RESXLS_COST_COST_EMBED_CHART"
    sdm.InitString RID_RESXLS_COST_COST_ERROR_TYPE, "RID_RESXLS_COST_COST_ERROR_TYPE"
    sdm.InitString RID_RESXLS_COST_COST_INFO, "RID_RESXLS_COST_COST_INFO"
    sdm.InitString RID_RESXLS_COST_COST_INVAL_PASS, "RID_RESXLS_COST_COST_INVAL_PASS"
    sdm.InitString RID_RESXLS_COST_COST_INVALID_WORKSHEET_NAME, "RID_RESXLS_COST_COST_INVALID_WORKSHEET_NAME"
    sdm.InitString RID_RESXLS_COST_Cost_of_4_min_per_line__takes_no_account_of_complexity_of_code__number_of_comment_lines_and_so_on, "RID_RESXLS_COST_Cost_of_4_min_per_line__takes_no_account_of_complexity_of_code__number_of_comment_lines_and_so_on"
    sdm.InitString RID_RESXLS_COST_Cost_per_Issue_or_Factor__min, "RID_RESXLS_COST_Cost_per_Issue_or_Factor__min"
    sdm.InitString RID_RESXLS_COST_COST_PIVOT, "RID_RESXLS_COST_COST_PIVOT"
    sdm.InitString RID_RESXLS_COST_COST_SHEET_CHART, "RID_RESXLS_COST_COST_SHEET_CHART"
    sdm.InitString RID_RESXLS_COST_Cost_variations_occur_because_of_the_use_of_macros_with_controls, "RID_RESXLS_COST_Cost_variations_occur_because_of_the_use_of_macros_with_controls"
    sdm.InitString RID_RESXLS_COST_COST_VBA_CONT_TYPE, "RID_RESXLS_COST_COST_VBA_CONT_TYPE"
    sdm.InitString RID_RESXLS_COST_COST_VBA_CONTROLS, "RID_RESXLS_COST_COST_VBA_CONTROLS"
    sdm.InitString RID_RESXLS_COST_COST_VBA_FORMS, "RID_RESXLS_COST_COST_VBA_FORMS"
    sdm.InitString RID_RESXLS_COST_COST_VBA_LOC, "RID_RESXLS_COST_COST_VBA_LOC"
    sdm.InitString RID_RESXLS_COST_COST_VBA_PROPS, "RID_RESXLS_COST_COST_VBA_PROPS"
    sdm.InitString RID_RESXLS_COST_Costs__min, "RID_RESXLS_COST_Costs__min"
    sdm.InitString RID_RESXLS_COST_Custom_Bullet_List, "RID_RESXLS_COST_Custom_Bullet_List"
    sdm.InitString RID_RESXLS_COST_Document_Custom_Properties, "RID_RESXLS_COST_Document_Custom_Properties"
    sdm.InitString RID_RESXLS_COST_Document_Parts_Protection, "RID_RESXLS_COST_Document_Parts_Protection"
    sdm.InitString RID_RESXLS_COST_Embedded_Chart, "RID_RESXLS_COST_Embedded_Chart"
    sdm.InitString RID_RESXLS_COST_embedded_objects, "RID_RESXLS_COST_embedded_objects"
    sdm.InitString RID_RESXLS_COST_Embedded_OLE_Object, "RID_RESXLS_COST_Embedded_OLE_Object"
    sdm.InitString RID_RESXLS_COST_Excel, "RID_RESXLS_COST_Excel"
    sdm.InitString RID_RESXLS_COST_Excel_Totals, "RID_RESXLS_COST_Excel_Totals"
    sdm.InitString RID_RESXLS_COST_External, "RID_RESXLS_COST_External"
    sdm.InitString RID_RESXLS_COST_External_References_In_Macro___Module__Class_or_UserForm, "RID_RESXLS_COST_External_References_In_Macro___Module__Class_or_UserForm"
    sdm.InitString RID_RESXLS_COST_Factor_Count, "RID_RESXLS_COST_Factor_Count"
    sdm.InitString RID_RESXLS_COST_Fields, "RID_RESXLS_COST_Fields"
    sdm.InitString RID_RESXLS_COST_Filters, "RID_RESXLS_COST_Filters"
    sdm.InitString RID_RESXLS_COST_Fixed_SO7, "RID_RESXLS_COST_Fixed_SO7"
    sdm.InitString RID_RESXLS_COST_Form_Field, "RID_RESXLS_COST_Form_Field"
    sdm.InitString RID_RESXLS_COST_Format, "RID_RESXLS_COST_Format"
    sdm.InitString RID_RESXLS_COST_Functions, "RID_RESXLS_COST_Functions"
    sdm.InitString RID_RESXLS_COST_HEADERFOOTER_WORKAROUND, "RID_RESXLS_COST_HEADERFOOTER_WORKAROUND"
    sdm.InitString RID_RESXLS_COST_Hours_Per_MD, "RID_RESXLS_COST_Hours_Per_MD"
    sdm.InitString RID_RESXLS_COST_Indexes_And_References, "RID_RESXLS_COST_Indexes_And_References"
    sdm.InitString RID_RESXLS_COST_Invalid_Password_Entered, "RID_RESXLS_COST_Invalid_Password_Entered"
    sdm.InitString RID_RESXLS_COST_Invalid_Worksheet_Name, "RID_RESXLS_COST_Invalid_Worksheet_Name"
    sdm.InitString RID_RESXLS_COST_Issue__18_of_the_24_table_border_styles_do_not_display_correctly, "RID_RESXLS_COST_Issue__18_of_the_24_table_border_styles_do_not_display_correctly"
    sdm.InitString RID_RESXLS_COST_Issue__256_sheet_limit_exceeded, "RID_RESXLS_COST_Issue__256_sheet_limit_exceeded"
    sdm.InitString RID_RESXLS_COST_Issue__32000_row_limit_exceeded, "RID_RESXLS_COST_Issue__32000_row_limit_exceeded"
    sdm.InitString RID_RESXLS_COST_Issue__A_worksheet_in_the_workbook_has_a_name_with_invalid_characters, "RID_RESXLS_COST_Issue__A_worksheet_in_the_workbook_has_a_name_with_invalid_characters"
    sdm.InitString RID_RESXLS_COST_Issue__Calls_to_windows_API_functions_are_windows_specific, "RID_RESXLS_COST_Issue__Calls_to_windows_API_functions_are_windows_specific"
    sdm.InitString RID_RESXLS_COST_Issue__Chart_support_is_limited_in_SO7, "RID_RESXLS_COST_Issue__Chart_support_is_limited_in_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__Comments_are_lost_on_migration_to_SO7, "RID_RESXLS_COST_Issue__Comments_are_lost_on_migration_to_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__Custom_Properties_are_lost_on_migration, "RID_RESXLS_COST_Issue__Custom_Properties_are_lost_on_migration"
    sdm.InitString RID_RESXLS_COST_Issue__Document_is_password_protected_and_cannot_be_opened_by_SO, "RID_RESXLS_COST_Issue__Document_is_password_protected_and_cannot_be_opened_by_SO"
    sdm.InitString RID_RESXLS_COST_Issue__External_functions_not_supported_by_SO7, "RID_RESXLS_COST_Issue__External_functions_not_supported_by_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__External_references_are_windows_specific, "RID_RESXLS_COST_Issue__External_references_are_windows_specific"
    sdm.InitString RID_RESXLS_COST_Issue__Fields_are_imported_but_need_to_have_the_link_to_datasource_reset, "RID_RESXLS_COST_Issue__Fields_are_imported_but_need_to_have_the_link_to_datasource_reset"
    sdm.InitString RID_RESXLS_COST_Issue__Graphics___frames_in_header_footer_tend_to_be_misplaced, "RID_RESXLS_COST_Issue__Graphics___frames_in_header_footer_tend_to_be_misplaced"
    sdm.InitString RID_RESXLS_COST_Issue__Incorrect_password_entered, "RID_RESXLS_COST_Issue__Incorrect_password_entered"
    sdm.InitString RID_RESXLS_COST_Issue__Information_relating_to_the_data_source_is_lost_in_migration, "RID_RESXLS_COST_Issue__Information_relating_to_the_data_source_is_lost_in_migration"
    sdm.InitString RID_RESXLS_COST_Issue__Invalid_password_entered_Rerun_analysis, "RID_RESXLS_COST_Issue__Invalid_password_entered_Rerun_analysis"
    sdm.InitString RID_RESXLS_COST_Issue__Lost_during_migration_No_StarOffice_equivalent, "RID_RESXLS_COST_Issue__Lost_during_migration_No_StarOffice_equivalent"
    sdm.InitString RID_RESXLS_COST_Issue__Macros_from_old_workbook_versions_do_not_import_into_SO7, "RID_RESXLS_COST_Issue__Macros_from_old_workbook_versions_do_not_import_into_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__Modules_will_need_porting_to_StarOffice_Basic, "RID_RESXLS_COST_Issue__Modules_will_need_porting_to_StarOffice_Basic"
    sdm.InitString RID_RESXLS_COST_Issue__Most_migrate_ok_but_do_not_roundtrip, "RID_RESXLS_COST_Issue__Most_migrate_ok_but_do_not_roundtrip"
    sdm.InitString RID_RESXLS_COST_Issue__Movies_are_not_supported_by_SO7, "RID_RESXLS_COST_Issue__Movies_are_not_supported_by_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__Need_to_set_these_to_be_included_in_TOC_in_SO7, "RID_RESXLS_COST_Issue__Need_to_set_these_to_be_included_in_TOC_in_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__Password_protection_of_comments__tracked_changes_and_forms_is_lost, "RID_RESXLS_COST_Issue__Password_protection_of_comments__tracked_changes_and_forms_is_lost"
    sdm.InitString RID_RESXLS_COST_Issue__Pivot_charts_are_not_supported_by_SO7, "RID_RESXLS_COST_Issue__Pivot_charts_are_not_supported_by_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__Presents_a_cross_platform_interoperability_issue, "RID_RESXLS_COST_Issue__Presents_a_cross_platform_interoperability_issue"
    sdm.InitString RID_RESXLS_COST_Issue__Print_wide_pages_not_supported, "RID_RESXLS_COST_Issue__Print_wide_pages_not_supported"
    sdm.InitString RID_RESXLS_COST_Issue__Shading_can_be_applied_to_fields_and_to_controls_separately_in_Word, "RID_RESXLS_COST_Issue__Shading_can_be_applied_to_fields_and_to_controls_separately_in_Word"
    sdm.InitString RID_RESXLS_COST_Issue__Table_imported_as_text_as_no_StarOffice_equivalent, "RID_RESXLS_COST_Issue__Table_imported_as_text_as_no_StarOffice_equivalent"
    sdm.InitString RID_RESXLS_COST_Issue__Tables_nested_in_tables_are_not_supported_by_SO7, "RID_RESXLS_COST_Issue__Tables_nested_in_tables_are_not_supported_by_SO7"
    sdm.InitString RID_RESXLS_COST_Issue__Tabs_are_incorrect_after_migration, "RID_RESXLS_COST_Issue__Tabs_are_incorrect_after_migration"
    sdm.InitString RID_RESXLS_COST_Issue__Text_form_fields_roundtrip_as_Fill_In_field, "RID_RESXLS_COST_Issue__Text_form_fields_roundtrip_as_Fill_In_field"
    sdm.InitString RID_RESXLS_COST_Issue__Unsupported_formatting_set, "RID_RESXLS_COST_Issue__Unsupported_formatting_set"
    sdm.InitString RID_RESXLS_COST_Issue__Unsupported_function_type, "RID_RESXLS_COST_Issue__Unsupported_function_type"
    sdm.InitString RID_RESXLS_COST_Issue__Userform_controls_will_need_porting_to_StarOffice_Basic, "RID_RESXLS_COST_Issue__Userform_controls_will_need_porting_to_StarOffice_Basic"
    sdm.InitString RID_RESXLS_COST_Issue__Userforms_controls_will_need_porting_to_StarOffice_Basic, "RID_RESXLS_COST_Issue__Userforms_controls_will_need_porting_to_StarOffice_Basic"
    sdm.InitString RID_RESXLS_COST_Issue__Userforms_will_need_porting_to_StarOffice_Basic, "RID_RESXLS_COST_Issue__Userforms_will_need_porting_to_StarOffice_Basic"
    sdm.InitString RID_RESXLS_COST_Issue__VBProject_is_password_protected_Remove_password__rerun_analysis, "RID_RESXLS_COST_Issue__VBProject_is_password_protected_Remove_password__rerun_analysis"
    sdm.InitString RID_RESXLS_COST_Issue__Will_need_refreshing_in_SO7_External_datasources_and_functions_are_not_supported, "RID_RESXLS_COST_Issue__Will_need_refreshing_in_SO7_External_datasources_and_functions_are_not_supported"
    sdm.InitString RID_RESXLS_COST_Issue__Workbook_protection_is_not_supported, "RID_RESXLS_COST_Issue__Workbook_protection_is_not_supported"
    sdm.InitString RID_RESXLS_COST_Issue__Writer_cannot_correctly_display_cells_spanning_more_than_1_page, "RID_RESXLS_COST_Issue__Writer_cannot_correctly_display_cells_spanning_more_than_1_page"
    sdm.InitString RID_RESXLS_COST_Issue_Category, "RID_RESXLS_COST_Issue_Category"
    sdm.InitString RID_RESXLS_COST_Issue_Count, "RID_RESXLS_COST_Issue_Count"
    sdm.InitString RID_RESXLS_COST_Issue_Type, "RID_RESXLS_COST_Issue_Type"
    sdm.InitString RID_RESXLS_COST_Linked_OLE_Object, "RID_RESXLS_COST_Linked_OLE_Object"
    sdm.InitString RID_RESXLS_COST_Mail_Merge_Datasource, "RID_RESXLS_COST_Mail_Merge_Datasource"
    sdm.InitString RID_RESXLS_COST_Mail_Merge_Field, "RID_RESXLS_COST_Mail_Merge_Field"
    sdm.InitString RID_RESXLS_COST_Major_Issue, "RID_RESXLS_COST_Major_Issue"
    sdm.InitString RID_RESXLS_COST_MAX_SHEETS_WORKAROUND, "RID_RESXLS_COST_MAX_SHEETS_WORKAROUND"
    sdm.InitString RID_RESXLS_COST_Maximum_Rows_Exceeded, "RID_RESXLS_COST_Maximum_Rows_Exceeded"
    sdm.InitString RID_RESXLS_COST_Maximum_Sheets_Exceeded, "RID_RESXLS_COST_Maximum_Sheets_Exceeded"
    sdm.InitString RID_RESXLS_COST_Migration_Issues_Costs, "RID_RESXLS_COST_Migration_Issues_Costs"
    sdm.InitString RID_RESXLS_COST_Movie, "RID_RESXLS_COST_Movie"
    sdm.InitString RID_RESXLS_COST_NESTED_TABLE_WORKAROUND, "RID_RESXLS_COST_NESTED_TABLE_WORKAROUND"
    sdm.InitString RID_RESXLS_COST_Nested_Tables, "RID_RESXLS_COST_Nested_Tables"
    sdm.InitString RID_RESXLS_COST_Not_Planned, "RID_RESXLS_COST_Not_Planned"
    sdm.InitString RID_RESXLS_COST_Notes_And_Handouts, "RID_RESXLS_COST_Notes_And_Handouts"
    sdm.InitString RID_RESXLS_COST_Number_Of_External_References, "RID_RESXLS_COST_Number_Of_External_References"
    sdm.InitString RID_RESXLS_COST_Number_Of_Lines, "RID_RESXLS_COST_Number_Of_Lines"
    sdm.InitString RID_RESXLS_COST_Number_of_Lines_in_Unique_Modules, "RID_RESXLS_COST_Number_of_Lines_in_Unique_Modules"
    sdm.InitString RID_RESXLS_COST_Number_of_Modules, "RID_RESXLS_COST_Number_of_Modules"
    sdm.InitString RID_RESXLS_COST_Number_of_Unique_Modules, "RID_RESXLS_COST_Number_of_Unique_Modules"
    sdm.InitString RID_RESXLS_COST_Numbering_Reference, "RID_RESXLS_COST_Numbering_Reference"
    sdm.InitString RID_RESXLS_COST_Numbering_Reference_Comment, "RID_RESXLS_COST_Numbering_Reference_Comment"
    sdm.InitString RID_RESXLS_COST_Object_In_Header_Footer, "RID_RESXLS_COST_Object_In_Header_Footer"
    sdm.InitString RID_RESXLS_COST_Objects_And_Graphics, "RID_RESXLS_COST_Objects_And_Graphics"
    sdm.InitString RID_RESXLS_COST_Objects_Graphics_And_Frames, "RID_RESXLS_COST_Objects_Graphics_And_Frames"
    sdm.InitString RID_RESXLS_COST_Objects_Graphics_And_Textboxes, "RID_RESXLS_COST_Objects_Graphics_And_Textboxes"
    sdm.InitString RID_RESXLS_COST_Old_Workbook_Version, "RID_RESXLS_COST_Old_Workbook_Version"
    sdm.InitString RID_RESXLS_COST_OLE_Control, "RID_RESXLS_COST_OLE_Control"
    sdm.InitString RID_RESXLS_COST_OLECONTR_COST, "RID_RESXLS_COST_OLECONTR_COST"
    sdm.InitString RID_RESXLS_COST_Page_Setup, "RID_RESXLS_COST_Page_Setup"
    sdm.InitString RID_RESXLS_COST_Password_Protected, "RID_RESXLS_COST_Password_Protected"
    sdm.InitString RID_RESXLS_COST_Password_Protection, "RID_RESXLS_COST_Password_Protection"
    sdm.InitString RID_RESXLS_COST_Pivot, "RID_RESXLS_COST_Pivot"
    sdm.InitString RID_RESXLS_COST_Planned_SO8, "RID_RESXLS_COST_Planned_SO8"
    sdm.InitString RID_RESXLS_COST_Portability, "RID_RESXLS_COST_Portability"
    sdm.InitString RID_RESXLS_COST_PowerPoint, "RID_RESXLS_COST_PowerPoint"
    sdm.InitString RID_RESXLS_COST_PowerPoint_Totals, "RID_RESXLS_COST_PowerPoint_Totals"
    sdm.InitString RID_RESXLS_COST_Prepared_Issue_Count, "RID_RESXLS_COST_Prepared_Issue_Count"
    sdm.InitString RID_RESXLS_COST_Prepared_Savings__min, "RID_RESXLS_COST_Prepared_Savings__min"
    sdm.InitString RID_RESXLS_COST_Properties___Module__Class_or_UserForm, "RID_RESXLS_COST_Properties___Module__Class_or_UserForm"
    sdm.InitString RID_RESXLS_COST_Remove_password_to_open, "RID_RESXLS_COST_Remove_password_to_open"
    sdm.InitString RID_RESXLS_COST_Remove_the_invalid_characters_from_the_worksheet_name, "RID_RESXLS_COST_Remove_the_invalid_characters_from_the_worksheet_name"
    sdm.InitString RID_RESXLS_COST_Rerun_analysis_with_correct_password, "RID_RESXLS_COST_Rerun_analysis_with_correct_password"
    sdm.InitString RID_RESXLS_COST_Reset_link_to_Data_Base_having_setup_a_Datasource_in_SO7, "RID_RESXLS_COST_Reset_link_to_Data_Base_having_setup_a_Datasource_in_SO7"
    sdm.InitString RID_RESXLS_COST_set_cost_factor_if_the_macro_has_to_be_ported_off_Windows, "RID_RESXLS_COST_set_cost_factor_if_the_macro_has_to_be_ported_off_Windows"
    sdm.InitString RID_RESXLS_COST_Save_workbook_as_a_new_workbook_version, "RID_RESXLS_COST_Save_workbook_as_a_new_workbook_version"
    sdm.InitString RID_RESXLS_COST_Setup_Datasource_in_SO7_using_Tools__Datasource, "RID_RESXLS_COST_Setup_Datasource_in_SO7_using_Tools__Datasource"
    sdm.InitString RID_RESXLS_COST_Sheet_Chart, "RID_RESXLS_COST_Sheet_Chart"
    sdm.InitString RID_RESXLS_COST_Status, "RID_RESXLS_COST_Status"
    sdm.InitString RID_RESXLS_COST_Table_Of_Authorities, "RID_RESXLS_COST_Table_Of_Authorities"
    sdm.InitString RID_RESXLS_COST_Table_Of_Authorities_Field, "RID_RESXLS_COST_Table_Of_Authorities_Field"
    sdm.InitString RID_RESXLS_COST_Table_Of_Contents, "RID_RESXLS_COST_Table_Of_Contents"
    sdm.InitString RID_RESXLS_COST_Tables, "RID_RESXLS_COST_Tables"
    sdm.InitString RID_RESXLS_COST_TOC_ISSUE_WORKAROUND, "RID_RESXLS_COST_TOC_ISSUE_WORKAROUND"
    sdm.InitString RID_RESXLS_COST_Total_Cost___MD, "RID_RESXLS_COST_Total_Cost___MD"
    sdm.InitString RID_RESXLS_COST_User_Error, "RID_RESXLS_COST_User_Error"
    sdm.InitString RID_RESXLS_COST_User_Forms_Control_Count, "RID_RESXLS_COST_User_Forms_Control_Count"
    sdm.InitString RID_RESXLS_COST_User_Forms_Control_Type_Count, "RID_RESXLS_COST_User_Forms_Control_Type_Count"
    sdm.InitString RID_RESXLS_COST_User_Forms_Count, "RID_RESXLS_COST_User_Forms_Count"
    sdm.InitString RID_RESXLS_COST_VBA_Macros, "RID_RESXLS_COST_VBA_Macros"
    sdm.InitString RID_RESXLS_COST_Word, "RID_RESXLS_COST_Word"
    sdm.InitString RID_RESXLS_COST_Word_Totals, "RID_RESXLS_COST_Word_Totals"
    sdm.InitString RID_RESXLS_COST_Work_Around, "RID_RESXLS_COST_Work_Around"
    sdm.InitString RID_RESXLS_COST_Workaround_Remove_protection_before_importing, "RID_RESXLS_COST_Workaround_Remove_protection_before_importing"
    sdm.InitString RID_RESXLS_COST_WorkbookProtection, "RID_RESXLS_COST_WorkbookProtection"
    sdm.InitString RID_RESXLS_DP_Accessed, "RID_RESXLS_DP_Accessed"
    sdm.InitString RID_RESXLS_DP_All_Analysed_Documents___Properties, "RID_RESXLS_DP_All_Analysed_Documents___Properties"
    sdm.InitString RID_RESXLS_DP_Application, "RID_RESXLS_DP_Application"
    sdm.InitString RID_RESXLS_DP_Based_on_Template, "RID_RESXLS_DP_Based_on_Template"
    sdm.InitString RID_RESXLS_DP_Costs, "RID_RESXLS_DP_Costs"
    sdm.InitString RID_RESXLS_DP_Created, "RID_RESXLS_DP_Created"
    sdm.InitString RID_RESXLS_DP_Document_Details, "RID_RESXLS_DP_Document_Details"
    sdm.InitString RID_RESXLS_DP_Document_Issue_Costs, "RID_RESXLS_DP_Document_Issue_Costs"
    sdm.InitString RID_RESXLS_DP_Document_Migration_Issues, "RID_RESXLS_DP_Document_Migration_Issues"
    sdm.InitString RID_RESXLS_DP_Document_Name, "RID_RESXLS_DP_Document_Name"
    sdm.InitString RID_RESXLS_DP_Document_Name_and_Path, "RID_RESXLS_DP_Document_Name_and_Path"
    sdm.InitString RID_RESXLS_DP_Issues_Complex_count, "RID_RESXLS_DP_Issues_Complex_count"
    sdm.InitString RID_RESXLS_DP_Last_Saved_By, "RID_RESXLS_DP_Last_Saved_By"
    sdm.InitString RID_RESXLS_DP_Lines_of_Macro_Code, "RID_RESXLS_DP_Lines_of_Macro_Code"
    sdm.InitString RID_RESXLS_DP_Macro_Issues, "RID_RESXLS_DP_Macro_Issues"
    sdm.InitString RID_RESXLS_DP_Macro_Migration_Costs, "RID_RESXLS_DP_Macro_Migration_Costs"
    sdm.InitString RID_RESXLS_DP_Macro_Migration_Issues, "RID_RESXLS_DP_Macro_Migration_Issues"
    sdm.InitString RID_RESXLS_DP_Migration_Issues, "RID_RESXLS_DP_Migration_Issues"
    sdm.InitString RID_RESXLS_DP_Minor_Issues, "RID_RESXLS_DP_Minor_Issues"
    sdm.InitString RID_RESXLS_DP_Modified, "RID_RESXLS_DP_Modified"
    sdm.InitString RID_RESXLS_DP_Pages_Sheets_Slides, "RID_RESXLS_DP_Pages_Sheets_Slides"
    sdm.InitString RID_RESXLS_DP_Prepareable_Issues_Costs, "RID_RESXLS_DP_Prepareable_Issues_Costs"
    sdm.InitString RID_RESXLS_DP_Prepared_Issues, "RID_RESXLS_DP_Prepared_Issues"
    sdm.InitString RID_RESXLS_DP_Printed, "RID_RESXLS_DP_Printed"
    sdm.InitString RID_RESXLS_DP_Revision, "RID_RESXLS_DP_Revision"
    sdm.InitString RID_RESXLS_DP_User_Form_Count, "RID_RESXLS_DP_User_Form_Count"
    sdm.InitString RID_RESXLS_ID_All_Documents_with_Issues___Issue_Details, "RID_RESXLS_ID_All_Documents_with_Issues___Issue_Details"
    sdm.InitString RID_RESXLS_ID_Application, "RID_RESXLS_ID_Application"
    sdm.InitString RID_RESXLS_ID_Column__Left, "RID_RESXLS_ID_Column__Left"
    sdm.InitString RID_RESXLS_ID_Document_Name, "RID_RESXLS_ID_Document_Name"
    sdm.InitString RID_RESXLS_ID_Document_Name_and_Path, "RID_RESXLS_ID_Document_Name_and_Path"
    sdm.InitString RID_RESXLS_ID_Issue_Category, "RID_RESXLS_ID_Issue_Category"
    sdm.InitString RID_RESXLS_ID_Issue_Details, "RID_RESXLS_ID_Issue_Details"
    sdm.InitString RID_RESXLS_ID_Issue_Type, "RID_RESXLS_ID_Issue_Type"
    sdm.InitString RID_RESXLS_ID_Line__Row__Top, "RID_RESXLS_ID_Line__Row__Top"
    sdm.InitString RID_RESXLS_ID_Location, "RID_RESXLS_ID_Location"
    sdm.InitString RID_RESXLS_ID_Location_Type, "RID_RESXLS_ID_Location_Type"
    sdm.InitString RID_RESXLS_OV_3_to_6_months, "RID_RESXLS_OV_3_to_6_months"
    sdm.InitString RID_RESXLS_OV_6_to_12_months, "RID_RESXLS_OV_6_to_12_months"
    sdm.InitString RID_RESXLS_OV_Complex, "RID_RESXLS_OV_Complex"
    sdm.InitString RID_RESXLS_OV_Cost_estimates_for_migration, "RID_RESXLS_OV_Cost_estimates_for_migration"
    sdm.InitString RID_RESXLS_OV_Costs, "RID_RESXLS_OV_Costs"
    sdm.InitString RID_RESXLS_OV_Docs_with_DocumentMigrationIssues, "RID_RESXLS_OV_Docs_with_DocumentMigrationIssues"
    sdm.InitString RID_RESXLS_OV_Document_Migration_Costs, "RID_RESXLS_OV_Document_Migration_Costs"
    sdm.InitString RID_RESXLS_OV_Document_Modification_Dates, "RID_RESXLS_OV_Document_Modification_Dates"
    sdm.InitString RID_RESXLS_OV_Document_Type, "RID_RESXLS_OV_Document_Type"
    sdm.InitString RID_RESXLS_OV_Documents_with_Macro_Migration_Issues, "RID_RESXLS_OV_Documents_with_Macro_Migration_Issues"
    sdm.InitString RID_RESXLS_OV_Excel, "RID_RESXLS_OV_Excel"
    sdm.InitString RID_RESXLS_OV_Excel_Costs, "RID_RESXLS_OV_Excel_Costs"
    sdm.InitString RID_RESXLS_OV_Excel_Spreadsheet, "RID_RESXLS_OV_Excel_Spreadsheet"
    sdm.InitString RID_RESXLS_OV_Excel_Template, "RID_RESXLS_OV_Excel_Template"
    sdm.InitString RID_RESXLS_OV_GREATER_THAN_1_year, "RID_RESXLS_OV_GREATER_THAN_1_year"
    sdm.InitString RID_RESXLS_OV_Last_Modified, "RID_RESXLS_OV_Last_Modified"
    sdm.InitString RID_RESXLS_OV_LESS_3_months, "RID_RESXLS_OV_LESS_3_months"
    sdm.InitString RID_RESXLS_OV_LESS_THAN3MONTHS, "RID_RESXLS_OV_LESS_THAN3MONTHS"
    sdm.InitString RID_RESXLS_OV_Macro_Migration_Costs, "RID_RESXLS_OV_Macro_Migration_Costs"
    sdm.InitString RID_RESXLS_OV_Medium, "RID_RESXLS_OV_Medium"
    sdm.InitString RID_RESXLS_OV_Minor, "RID_RESXLS_OV_Minor"
    sdm.InitString RID_RESXLS_OV_None, "RID_RESXLS_OV_None"
    sdm.InitString RID_RESXLS_OV_Number, "RID_RESXLS_OV_Number"
    sdm.InitString RID_RESXLS_OV_Number_of_Documents_Analyzed, "RID_RESXLS_OV_Number_of_Documents_Analyzed"
    sdm.InitString RID_RESXLS_OV_NumberOfDocs, "RID_RESXLS_OV_NumberOfDocs"
    sdm.InitString RID_RESXLS_OV_Potential_savings_macro_conversion, "RID_RESXLS_OV_Potential_savings_macro_conversion"
    sdm.InitString RID_RESXLS_OV_Potential_savings_prepared_docs, "RID_RESXLS_OV_Potential_savings_prepared_docs"
    sdm.InitString RID_RESXLS_OV_PowerPoint, "RID_RESXLS_OV_PowerPoint"
    sdm.InitString RID_RESXLS_OV_PowerPoint_Costs, "RID_RESXLS_OV_PowerPoint_Costs"
    sdm.InitString RID_RESXLS_OV_PowerPoint_Document, "RID_RESXLS_OV_PowerPoint_Document"
    sdm.InitString RID_RESXLS_OV_PowerPoint_Template, "RID_RESXLS_OV_PowerPoint_Template"
    sdm.InitString RID_RESXLS_OV_Simple, "RID_RESXLS_OV_Simple"
    sdm.InitString RID_RESXLS_OV_Total, "RID_RESXLS_OV_Total"
    sdm.InitString RID_RESXLS_OV_Totals, "RID_RESXLS_OV_Totals"
    sdm.InitString RID_RESXLS_OV_Word, "RID_RESXLS_OV_Word"
    sdm.InitString RID_RESXLS_OV_Word_Costs, "RID_RESXLS_OV_Word_Costs"
    sdm.InitString RID_RESXLS_OV_Word_Document, "RID_RESXLS_OV_Word_Document"
    sdm.InitString RID_RESXLS_OV_Word_Template, "RID_RESXLS_OV_Word_Template"
    sdm.InitString RID_RESXLS_PP_Action_Settings, "RID_RESXLS_PP_Action_Settings"
    sdm.InitString RID_RESXLS_PP_Application, "RID_RESXLS_PP_Application"
    sdm.InitString RID_RESXLS_PP_Content___Document_Properties, "RID_RESXLS_PP_Content___Document_Properties"
    sdm.InitString RID_RESXLS_PP_Document_Name_and_Path, "RID_RESXLS_PP_Document_Name_and_Path"
    sdm.InitString RID_RESXLS_PP_Fields, "RID_RESXLS_PP_Fields"
    sdm.InitString RID_RESXLS_PP_Format, "RID_RESXLS_PP_Format"
    sdm.InitString RID_RESXLS_PP_Notes___Handouts, "RID_RESXLS_PP_Notes___Handouts"
    sdm.InitString RID_RESXLS_PP_Objects__Graphics___Textboxes, "RID_RESXLS_PP_Objects__Graphics___Textboxes"
    sdm.InitString RID_RESXLS_PP_Portability, "RID_RESXLS_PP_Portability"
    sdm.InitString RID_RESXLS_PP_PowerPoint_Documents_with_Issues___Issue_Summary, "RID_RESXLS_PP_PowerPoint_Documents_with_Issues___Issue_Summary"
    sdm.InitString RID_RESXLS_PP_PowerPoint_Presentation_Name, "RID_RESXLS_PP_PowerPoint_Presentation_Name"
    sdm.InitString RID_RESXLS_PP_VBA_Macros, "RID_RESXLS_PP_VBA_Macros"
    sdm.InitString RID_RESXLS_RD_All_Documents_with_Macros___Reference_Details, "RID_RESXLS_RD_All_Documents_with_Macros___Reference_Details"
    sdm.InitString RID_RESXLS_RD_Application, "RID_RESXLS_RD_Application"
    sdm.InitString RID_RESXLS_RD_Description, "RID_RESXLS_RD_Description"
    sdm.InitString RID_RESXLS_RD_Document_Name, "RID_RESXLS_RD_Document_Name"
    sdm.InitString RID_RESXLS_RD_Document_Name_and_Path, "RID_RESXLS_RD_Document_Name_and_Path"
    sdm.InitString RID_RESXLS_RD_Location, "RID_RESXLS_RD_Location"
    sdm.InitString RID_RESXLS_RD_Reference, "RID_RESXLS_RD_Reference"
    sdm.InitString RID_RESXLS_RD_Reference_Details, "RID_RESXLS_RD_Reference_Details"
    sdm.InitString RID_RESXLS_WI_Application, "RID_RESXLS_WI_Application"
    sdm.InitString RID_RESXLS_WI_Changes_and_Reviewing, "RID_RESXLS_WI_Changes_and_Reviewing"
    sdm.InitString RID_RESXLS_WI_Content___Document_Properties, "RID_RESXLS_WI_Content___Document_Properties"
    sdm.InitString RID_RESXLS_WI_Controls, "RID_RESXLS_WI_Controls"
    sdm.InitString RID_RESXLS_WI_Document_Name_and_Path, "RID_RESXLS_WI_Document_Name_and_Path"
    sdm.InitString RID_RESXLS_WI_Fields, "RID_RESXLS_WI_Fields"
    sdm.InitString RID_RESXLS_WI_Format, "RID_RESXLS_WI_Format"
    sdm.InitString RID_RESXLS_WI_Index_and_References, "RID_RESXLS_WI_Index_and_References"
    sdm.InitString RID_RESXLS_WI_Objects__Graphics____Frames, "RID_RESXLS_WI_Objects__Graphics____Frames"
    sdm.InitString RID_RESXLS_WI_Portability, "RID_RESXLS_WI_Portability"
    sdm.InitString RID_RESXLS_WI_Tables, "RID_RESXLS_WI_Tables"
    sdm.InitString RID_RESXLS_WI_VBA_Macros, "RID_RESXLS_WI_VBA_Macros"
    sdm.InitString RID_RESXLS_WI_Word_Document_Name, "RID_RESXLS_WI_Word_Document_Name"
    sdm.InitString RID_RESXLS_WI_Word_Documents_with_Issues___Issue_Summary, "RID_RESXLS_WI_Word_Documents_with_Issues___Issue_Summary"
    sdm.InitString RID_RESXLS_XL_Application, "RID_RESXLS_XL_Application"
    sdm.InitString RID_RESXLS_XL_Changes___Reviewing, "RID_RESXLS_XL_Changes___Reviewing"
    sdm.InitString RID_RESXLS_XL_Charts___Tables, "RID_RESXLS_XL_Charts___Tables"
    sdm.InitString RID_RESXLS_XL_Excel_Documents_with_Issues___Issue_Summary, "RID_RESXLS_XL_Excel_Documents_with_Issues___Issue_Summary"
    sdm.InitString RID_RESXLS_XL_Excel_Spreadsheet_Name, "RID_RESXLS_XL_Excel_Spreadsheet_Name"
    sdm.InitString RID_RESXLS_XL_Format, "RID_RESXLS_XL_Format"
    sdm.InitString RID_RESXLS_XL_Functions, "RID_RESXLS_XL_Functions"
    sdm.InitString RID_RESXLS_XL_Objects___Graphics, "RID_RESXLS_XL_Objects___Graphics"

    sdm.InitString RID_RESXLS_COST_DB_Query, "RID_RESXLS_COST_DB_Query"
    sdm.InitString RID_RESXLS_COST_DB_Query_Comment, "RID_RESXLS_COST_DB_Query_Comment"
    sdm.InitString RID_RESXLS_COST_LineStyle, "RID_RESXLS_COST_LineStyle"
    sdm.InitString RID_RESXLS_COST_LineStyle_Comment, "RID_RESXLS_COST_LineStyle_Comment"
    sdm.InitString RID_RESXLS_COST_Numbering, "RID_RESXLS_COST_Numbering"
    sdm.InitString RID_RESXLS_COST_Numbering_Comment, "RID_RESXLS_COST_Numbering_Comment"
    sdm.InitString RID_RESXLS_COST_Template, "RID_RESXLS_COST_Template"
    sdm.InitString RID_RESXLS_COST_Template_Comment, "RID_RESXLS_COST_Template_Comment"
    sdm.InitString RID_RESXLS_COST_DATEDIF_Note, "RID_RESXLS_COST_DATEDIF_Note"
    sdm.InitString RID_RESXLS_COST_Tabstop, "RID_RESXLS_COST_Tabstop"
    sdm.InitString RID_RESXLS_COST_Tabstop_Comment, "RID_RESXLS_COST_Tabstop_Comment"
    sdm.InitString RID_RESXLS_COST_Transparent, "RID_RESXLS_COST_Transparent"
    sdm.InitString RID_RESXLS_COST_Transparent_Comment, "RID_RESXLS_COST_Transparent_Comment"
    sdm.InitString RID_RESXLS_COST_GradientStyle, "RID_RESXLS_COST_GradientStyle"
    sdm.InitString RID_RESXLS_COST_GradientStyle_Comment, "RID_RESXLS_COST_GradientStyle_Comment"
    sdm.InitString RID_RESXLS_COST_GradientStyle_Note, "RID_RESXLS_COST_GradientStyle_Note"
    sdm.InitString RID_RESXLS_COST_Hyperlink, "RID_RESXLS_COST_Hyperlink"
    sdm.InitString RID_RESXLS_COST_Hyperlink_Comment, "RID_RESXLS_COST_Hyperlink_Comment"
    sdm.InitString RID_RESXLS_COST_HyperlinkSplit, "RID_RESXLS_COST_HyperlinkSplit"
    sdm.InitString RID_RESXLS_COST_HyperlinkSplit_Comment, "RID_RESXLS_COST_HyperlinkSplit_Comment"
    sdm.InitString RID_RESXLS_COST_Background, "RID_RESXLS_COST_Background"
    sdm.InitString RID_RESXLS_COST_Background_Comment, "RID_RESXLS_COST_Background_Comment"

End Sub
