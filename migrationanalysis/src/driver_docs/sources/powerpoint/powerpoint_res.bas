Attribute VB_Name = "powerpoint_res"
'/*************************************************************************
' *
' * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' * 
' * Copyright 2008 by Sun Microsystems, Inc.
' *
' * OpenOffice.org - a multi-platform office productivity suite
' *
' * $RCSfile: powerpoint_res.bas,v $
' *
' * This file is part of OpenOffice.org.
' *
' * OpenOffice.org is free software: you can redistribute it and/or modify
' * it under the terms of the GNU Lesser General Public License version 3
' * only, as published by the Free Software Foundation.
' *
' * OpenOffice.org is distributed in the hope that it will be useful,
' * but WITHOUT ANY WARRANTY; without even the implied warranty of
' * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' * GNU Lesser General Public License version 3 for more details
' * (a copy is included in the LICENSE file that accompanied this code).
' *
' * You should have received a copy of the GNU Lesser General Public License
' * version 3 along with OpenOffice.org.  If not, see
' * <http://www.openoffice.org/license.html>
' * for a copy of the LGPLv3 License.
' *
' ************************************************************************/

Public RID_STR_PP_TRUE As String
Public RID_STR_PP_FALSE As String
Public RID_STR_PP_ISSUE_OBJECTS_GRAPHICS_AND_TEXTBOXES As String
Public RID_STR_PP_SUBISSUE_COMMENT As String
Public RID_STR_PP_SUBISSUE_MOVIE As String
Public RID_STR_PP_SUBISSUE_BACKGROUND_NOTE As String
Public RID_STR_PP_SUBISSUE_NUMBERING_NOTE As String
Public RID_STR_PP_SUBISSUE_HYPERLINK_NOTE As String
Public RID_STR_PP_SUBISSUE_HYPERLINK_SPLIT_NOTE As String
Public RID_STR_PP_SUBISSUE_TEMPLATE As String
Public RID_STR_PP_SUBISSUE_TEMPLATE_NOTE As String
Public RID_STR_PP_SUBISSUE_TABSTOP_NOTE As String
Public RID_STR_PP_SUBISSUE_FONTS As String
Public RID_STR_PP_SUBISSUE_FONTS_NOTE As String
Public RID_STR_PP_ATTRIBUTE_CONTENT As String
Public RID_STR_PP_ATTRIBUTE_LOOP As String
Public RID_STR_PP_ATTRIBUTE_PLAYONENTRY As String
Public RID_STR_PP_ATTRIBUTE_REWIND As String
Public RID_STR_PP_ATTRIBUTE_TYPES As String
Public RID_STR_PP_ENUMERATION_VIEW_HANDOUT_MASTER As String
Public RID_STR_PP_ENUMERATION_VIEW_NORMAL As String
Public RID_STR_PP_ENUMERATION_VIEW_NOTES_MASTER As String
Public RID_STR_PP_ENUMERATION_VIEW_NOTES_PAGE As String
Public RID_STR_PP_ENUMERATION_VIEW_OUTLINE As String
Public RID_STR_PP_ENUMERATION_VIEW_SLIDE As String
Public RID_STR_PP_ENUMERATION_VIEW_SLIDE_MASTER As String
Public RID_STR_PP_ENUMERATION_VIEW_SLIDE_SORTER As String
Public RID_STR_PP_ENUMERATION_VIEW_TITLE_MASTER As String
Public RID_STR_PP_ENUMERATION_UNKNOWN As String

'Driver strings
Public RID_STR_DVR_PP_TXT2 As String
Public RID_STR_DVR_PP_TXT3 As String
Public RID_STR_DVR_PP_TXT4 As String
Public RID_STR_DVR_PP_TXT5 As String
Public RID_STR_DVR_PP_TXT6 As String
Public RID_STR_DVR_PP_TXT7 As String
Public RID_STR_DVR_PP_TXT8 As String

Public Sub LoadPPStrings(sdm As StringDataManager)
    sdm.InitString RID_STR_PP_TRUE, "RID_STR_PP_TRUE"
    sdm.InitString RID_STR_PP_FALSE, "RID_STR_PP_FALSE"
    sdm.InitString RID_STR_PP_ISSUE_OBJECTS_GRAPHICS_AND_TEXTBOXES, "RID_STR_PP_ISSUE_OBJECTS_GRAPHICS_AND_TEXTBOXES"
    sdm.InitString RID_STR_PP_SUBISSUE_COMMENT, "RID_STR_PP_SUBISSUE_COMMENT"
    sdm.InitString RID_STR_PP_SUBISSUE_MOVIE, "RID_STR_PP_SUBISSUE_MOVIE"
    sdm.InitString RID_STR_PP_SUBISSUE_BACKGROUND_NOTE, "RID_STR_PP_SUBISSUE_BACKGROUND_NOTE"
    sdm.InitString RID_STR_PP_SUBISSUE_NUMBERING_NOTE, "RID_STR_PP_SUBISSUE_NUMBERING_NOTE"
    sdm.InitString RID_STR_PP_SUBISSUE_HYPERLINK_NOTE, "RID_STR_PP_SUBISSUE_HYPERLINK_NOTE"
    sdm.InitString RID_STR_PP_SUBISSUE_HYPERLINK_SPLIT_NOTE, "RID_STR_PP_SUBISSUE_HYPERLINK_SPLIT_NOTE"
    sdm.InitString RID_STR_PP_SUBISSUE_TEMPLATE, "RID_STR_PP_SUBISSUE_TEMPLATE"
    sdm.InitString RID_STR_PP_SUBISSUE_TEMPLATE_NOTE, "RID_STR_PP_SUBISSUE_TEMPLATE_NOTE"
    sdm.InitString RID_STR_PP_SUBISSUE_TABSTOP_NOTE, "RID_STR_PP_SUBISSUE_TABSTOP_NOTE"
    sdm.InitString RID_STR_PP_SUBISSUE_FONTS, "RID_STR_PP_SUBISSUE_FONTS"
    sdm.InitString RID_STR_PP_SUBISSUE_FONTS_NOTE, "RID_STR_PP_SUBISSUE_FONTS_NOTE"
    sdm.InitString RID_STR_PP_ATTRIBUTE_CONTENT, "RID_STR_PP_ATTRIBUTE_CONTENT"
    sdm.InitString RID_STR_PP_ATTRIBUTE_LOOP, "RID_STR_PP_ATTRIBUTE_LOOP"
    sdm.InitString RID_STR_PP_ATTRIBUTE_PLAYONENTRY, "RID_STR_PP_ATTRIBUTE_PLAYONENTRY"
    sdm.InitString RID_STR_PP_ATTRIBUTE_REWIND, "RID_STR_PP_ATTRIBUTE_REWIND"
    sdm.InitString RID_STR_PP_ATTRIBUTE_TYPES, "RID_STR_PP_ATTRIBUTE_TYPES"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_HANDOUT_MASTER, "RID_STR_PP_ENUMERATION_VIEW_HANDOUT_MASTER"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_NORMAL, "RID_STR_PP_ENUMERATION_VIEW_NORMAL"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_NOTES_MASTER, "RID_STR_PP_ENUMERATION_VIEW_NOTES_MASTER"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_NOTES_PAGE, "RID_STR_PP_ENUMERATION_VIEW_NOTES_PAGE"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_OUTLINE, "RID_STR_PP_ENUMERATION_VIEW_OUTLINE"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_SLIDE, "RID_STR_PP_ENUMERATION_VIEW_SLIDE"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_SLIDE_MASTER, "RID_STR_PP_ENUMERATION_VIEW_SLIDE_MASTER"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_SLIDE_SORTER, "RID_STR_PP_ENUMERATION_VIEW_SLIDE_SORTER"
    sdm.InitString RID_STR_PP_ENUMERATION_VIEW_TITLE_MASTER, "RID_STR_PP_ENUMERATION_VIEW_TITLE_MASTER"
    sdm.InitString RID_STR_PP_ENUMERATION_UNKNOWN, "RID_STR_PP_ENUMERATION_UNKNOWN"
    
    'Driver strings
    sdm.InitString RID_STR_DVR_PP_TXT2, "RID_STR_DVR_PP_TXT2"
    sdm.InitString RID_STR_DVR_PP_TXT3, "RID_STR_DVR_PP_TXT3"
    sdm.InitString RID_STR_DVR_PP_TXT4, "RID_STR_DVR_PP_TXT4"
    sdm.InitString RID_STR_DVR_PP_TXT5, "RID_STR_DVR_PP_TXT5"
    sdm.InitString RID_STR_DVR_PP_TXT6, "RID_STR_DVR_PP_TXT6"
    sdm.InitString RID_STR_DVR_PP_TXT7, "RID_STR_DVR_PP_TXT7"
    sdm.InitString RID_STR_DVR_PP_TXT8, "RID_STR_DVR_PP_TXT8"
End Sub
