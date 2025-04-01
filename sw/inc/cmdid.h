/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once


#include <sfx2/sfxsids.hrc>
#include <svx/svxids.hrc>
#include <svl/solar.hrc>

class SvxFrameDirectionItem;
class SvxSizeItem;
class SwAddPrinterItem;
class SwPageFootnoteInfoItem;
class SwPtrItem;
class SwUINumRuleItem;

                 SID_SW_START
//                 (SID_SW_START +  100) defined in svxids.hrc
                 (SID_SW_START +  200) // 20200
//               (SID_SW_START +  300) defined in svxids.hrc
//               (SID_SW_START +  400) defined in svxids.hrc
//                (SID_SW_START +  600) defined in svxids.hrc
//            (SID_SW_START +  900) defined in svxids.hrc
                (SID_SW_START + 1000)
               (SID_SW_START + 1050)
                (SID_SW_START + 1100)
                 (SID_SW_START + 1180)
//            (SID_SW_START + 1250) defined in svxids.hrc
                (SID_SW_START + 1300)
              (SID_SW_START + 1400)
              (SID_SW_START + 1600)
//                (SID_SW_START + 1800) defined in sfxsids.hrc
               (SID_SW_START + 2000)
               (SID_SW_START + 2200)
               (SID_SW_START + 2400)
                (SID_SW_START + 2500)
              (SID_SW_START + 2550)
         (SID_SW_START + 2600)
            (SID_SW_START + 2610)

 /* More accurately, this range should be from FN_EXTRA2 to FN_PARAM2-1, but
 * FN_NUMBER_NEWSTART comes from FN_FORMAT2, and FN_PARAM_LINK_DISPLAY_NAME
 * comes from FN_PARAM2 so we need to include FORMAT2,
 * EDIT2 and QUERY2 and PARAM2 in the range...hopefully this will be fixed
 * soon */

      FN_FORMAT2
        (FN_PARAM2 + 199)

// Region: File
       (FN_FILE + 4 )    /* Create Global Document */
            (FN_FILE + 7 )    /* Open */

   (FN_FILE + 36)  /* Send outline to impress */
 (FN_FILE + 37)  /* Send outline to clipboard */

         (FN_FILE + 40 ) /* Create HTML document */

// Region: Edit
           (FN_EDIT + 4 )    /* Edit field */
        (FN_EDIT + 9 )    /* Edit link dialog */

       (FN_EDIT + 21)    /* Bullets */
     (FN_EDIT + 22 )   /* Update selected field */
   (FN_EDIT + 23)    /* Edit Index-Entry */
        (FN_EDIT + 26)    /* Update fields */
   (FN_EDIT + 27)    /* Execute macrofield */
         TypedWhichId<SfxStringItem>(FN_EDIT + 28)    /* Edit formula in RibbonBar */
           (FN_EDIT + 29)

// Region: Bullets
      (FN_EDIT + 30)  /* Bullet down */
        (FN_EDIT + 31)  /* Bullet up */
      (FN_EDIT + 32)  /* to previous bullet */
      (FN_EDIT + 33)  /* to next bullet */
    (FN_EDIT + 34)  /* move bullet up */
  (FN_EDIT + 35)  /* move bullet down */
     (FN_EDIT + 36)  /* bullet without number */
#pragma once //in SVX already
       (FN_EDIT + 37)  /* turn off numbering */
#endif

      (FN_EDIT + 39)  /* reduce in rank with sub points */
        (FN_EDIT + 40)  /* increase in rank with sub-items */
    (FN_EDIT + 41)  /* Push up with sub-items */
  (FN_EDIT + 42)  /* Push down with sub-items */
           (FN_EDIT + 43)  /* Update input fields */

   (FN_EDIT + 47)  /* go to next inputfield */
   (FN_EDIT + 48)  /* go to previous inputfield    */
            (FN_EDIT + 49)  /* go to bookmark by name */

        (FN_EDIT + 50)  /* Search again */

           (FN_EDIT + 61)  /* Force repagination */
        (FN_EDIT + 62)  /* Edit footnote */

          (FN_EDIT + 65)  /* Edit region */
       (FN_EDIT + 66)  /* From Refmark to the reference */
  (FN_EDIT + 67)  /* Edit current region*/

// already defined in SVX
        (FN_EDIT + 68)  /*  */
        (FN_EDIT + 69)  /*  */

                (FN_EDIT + 80)  /**/

               (FN_EDIT + 97) /* select paragraph*/

          (FN_EDIT2 + 24) /* update all links */

              (FN_EDIT2 + 26) /* show Redlining */
           (FN_EDIT2 + 27) /* comment Redlining */

                (FN_EDIT2 + 28) /* FN_UPDATE_ALL_LINKS,
                                                        FN_UPDATE_FIELDS,
                                                        FN_UPDATE_TOX,
                                                        CalcLayout */

            (FN_EDIT2 + 29) /* Redlining Accept/Reject */
              (FN_EDIT2 + 31) /* SlotId for SwFormatCol */
          (FN_EDIT2 + 32) /* edit current index */
       (FN_EDIT2 + 33) /* edit authorities entry*/
             (FN_EDIT2 + 34) /* update all charts */
// free
              (FN_EDIT2 + 36)    /* continue previous numbering */
     (FN_EDIT2 + 37)    /* accept redline at current position*/
     (FN_EDIT2 + 38)    /* reject redline at current position*/
  (FN_EDIT2 + 39)    /* reinstate redline at current position*/
  (FN_EDIT2 + 40)    /* reinstate redline and jump to next */
       (FN_EDIT2 + 41)    /* Go to the next change */
       (FN_EDIT2 + 42)    /* Go to the previous change */
        (FN_EDIT2 + 43)    /* Redlining Accept All*/
        (FN_EDIT2 + 44)    /* Redlining Reject All*/
     (FN_EDIT2 + 45)    /* Redlining Accept and jump to next*/
     (FN_EDIT2 + 46)    /* Redlining Reject and jump to next*/
 (FN_EDIT2 + 47) /* overwrite text of content control, and more*/
                (FN_EDIT2 + 48)    /* show field content in readonly documents to copy content*/
         (FN_EDIT2 + 49)    /* convert selected field to text */
     (FN_EDIT2 + 50)    /* redlining reinstate all */

// Region: View
        TypedWhichId<SfxInt16Item>(FN_VIEW + 3)   /* Draw wrapping dlg */
                (FN_VIEW + 11)  /* Horizontal ruler */

      (FN_VIEW + 12)  /* View text, section, table boundaries */
         (FN_VIEW + 13)  /* View graphic */
          (FN_VIEW + 14)  /* View bounds */
          (FN_VIEW + 15)  /* View fields */
              (FN_VIEW + 16)  /* Vertical Liner */
           (FN_VIEW + 17)  /* Vertical Scrollbar */
           (FN_VIEW + 18)  /* Horizontal Scrollbar */
 (FN_VIEW + 19)  /* View section boundaries */
//  slot number 20220 conflicts with FN_BUL_NUM_RULE_INDEX!

      (FN_VIEW + 24)  /* View meta chars */
           (FN_VIEW + 25)  /* View marks */
       (FN_VIEW + 26)  /* View field names */
       (FN_VIEW + 27)  /* View tablegrid */

    (FN_VIEW + 28)

             (FN_VIEW + 29)  /* Set page template to paragraph */

         (FN_VIEW + 37)  /* print layout */

    (FN_VIEW + 40)  /* Navigation Controller */

     (FN_VIEW + 42)  /* View hidden paragraphs */
   (FN_VIEW + 43)

         TypedWhichId<SfxUInt16Item>(FN_VIEW + 51)  /* create table controller for zoom */

  (FN_VIEW + 52) /* Number recognition in tables */
       TypedWhichId<SfxUInt16Item>(FN_VIEW + 53) /* Metric horizontal scrollbar */
       TypedWhichId<SfxUInt16Item>(FN_VIEW + 54) /* Metric vertical scrollbar */

          TypedWhichId<SfxUInt32Item>(FN_VIEW + 55)  /* Navigate By drop down controller */
          (FN_VIEW + 57)  /* uno:ScrollToPrevious */
          (FN_VIEW + 58)  /* uno:ScrollToNext */
 (FN_VIEW + 59)  /* Hide header, footer, and pagebreak */
  (FN_VIEW + 60)  /* Show tooltips for tracked changes */
 (FN_VIEW + 61)  /* Show advanced header/footer menu */
 (FN_VIEW + 62)  /* Show header, footer, and pagebreak */

 (FN_VIEW + 63)  /* Show outline content visibility toggle button */
   (FN_VIEW + 64)
              (FN_VIEW + 65)  /* Show tracked deletions in margin */
           (FN_VIEW + 66)  /* Menu for the next layout modes */
         (FN_VIEW + 67)  /* Show tracked deletions and insertions in text */
     (FN_VIEW + 68)  /* Show final text (deletions in margin) */
    (FN_VIEW + 69)  /* Show original text (insertions in margin) */
                (FN_VIEW + 70)
         (FN_VIEW + 71)  /* Menu for the track changes record modes */
          (FN_VIEW + 72)  /* Record track changes only in this view */
          (FN_VIEW + 73)  /* Record track changes only in all views */

// Region: Insert
      (FN_INSERT + 2 )  /* Bookmark */
// FN_INSERT + 3 is FN_INSERT_BREAK

     (FN_INSERT + 4 )  /* Break */
  (FN_INSERT + 5 )  /* Column break */
         TypedWhichId<SfxStringItem>(FN_INSERT + 8 )  /* Textcommand */
       (FN_INSERT + 9 )  /* Change the database field */

       (FN_INSERT + 10)  /* Caption */
       TypedWhichId<SfxStringItem>(FN_INSERT + 11)  /* Insert database field - for recording */
  (FN_INSERT + 12)  /* Footnote Dialog */

     (FN_INSERT + 13)  /* Insert Reference Field */

     (FN_INSERT + 18)    /* Newline */
 (FN_INSERT + 19)  /* Field dialog for mail merge*/
 (FN_INSERT + 20)  /* Rich text content control */
 (FN_INSERT + 21)  /* Checkbox content control */
    (FN_INSERT + 22)    /* Object */
     (FN_INSERT + 23)    /* Page break*/
 (FN_INSERT + 24)  /* Dropdown content control */
 (FN_INSERT + 25)  /* Content control properties */
 (FN_INSERT + 26) /* Picture content control */
 (FN_INSERT + 27) /* Date content control */
 (FN_INSERT + 28) /* Plain text content control */
               (FN_INSERT + 29)    /* Insert/edit PostIt */
         TypedWhichId<SfxStringItem>(FN_INSERT + 30)    /* Insert Table */
        TypedWhichId<SfxStringItem>(FN_INSERT+31)
 (FN_INSERT + 32) /* Combo box content control */
 (FN_INSERT + 33)   /* Insert interactive frame */
         TypedWhichId<SfxUInt16Item>(FN_INSERT + 34)    /* Insert Frame */

 (FN_INSERT + 35)    /* insert index entry */
 (FN_INSERT + 36) /*insert interactive non column frame*/
 TypedWhichId<SfxUInt16Item>(FN_INSERT + 37) /* Unfloat Frame */

     (FN_INSERT + 50)    /* anchor Draw object to page */
 (FN_INSERT + 51)   /* anchor Draw object to paragraph */
      (FN_INSERT + 52)    /* change hierarchy */
      (FN_INSERT + 53)    /* page number wizard */

     (FN_INSERT + 64)    /* mail merge wizard */
    (FN_INSERT + 66)    /* anchor Draw-Object to frame*/
            (FN_INSERT + 67)    /* insert record (serial letter) */
           (FN_INSERT + 69)    /* insert StarMath  */

 (FN_INSERT + 70)   /* mail merge toolbar - go to the first entry */
 (FN_INSERT + 71)    /* mail merge toolbar - go to the previous entry */
 (FN_INSERT + 72)    /* mail merge toolbar - go to the next entry */
 (FN_INSERT + 73)    /* mail merge toolbar - go to the next entry */
 (FN_INSERT + 74) /* mail merge toolbar - show or change the current entry */
 (FN_INSERT + 75) /* mail merge toolbar - checkbox to exclude the current entry */

    (FN_INSERT + 76)    /* position DrawText */

 (FN_INSERT + 77) /* mail merge toolbar - create the merged document */
 (FN_INSERT + 78) /* mail merge toolbar - save merged documents */
 (FN_INSERT + 79) /* mail merge toolbar - print merged documents */
 (FN_INSERT + 80) /* mail merge toolbar - email merged documents */

     (FN_INSERT + 82)    /* anchor Draw object to character */

           (FN_INSERT + 83)    /* insert record selection in to text */
      (FN_INSERT + 84)    /* insert record selection into fields */
     (FN_INSERT + 86)    /* insert database field */

          (FN_INSERT + 87)    /* toolbar controller insert*/
      (FN_INSERT + 88)    /* toolbar controller (insert/object) */
    (FN_INSERT + 89)    /* toolbar controller insert/field commands */

      (FN_INSERT + 90)
  (FN_INSERT + 91)
      (FN_INSERT + 92)
  (FN_INSERT + 93)
  (FN_INSERT + 94)
   (FN_INSERT + 95)
     (FN_INSERT + 96)
     (FN_INSERT + 97)
    (FN_INSERT + 98)
      (FN_INSERT + 99)

         (FN_INSERT + 100)

// Region: Paste (Part 2)
 (FN_INSERT2 + 5)  /* child window provided by mailmerge */

       (FN_INSERT2 + 7)    /* insert records into fields */
       (FN_INSERT2 + 8)    /* insert records into text */
        (FN_INSERT2 + 9)    /* serial letter print */
             TypedWhichId<SfxStringItem>(FN_INSERT2 + 10)   /* edit script field */
  (FN_INSERT2 + 12)   /* anchor object to character*/

    TypedWhichId<SfxStringListItem>(FN_INSERT2 + 13)   /* insert default header */
    TypedWhichId<SfxStringListItem>(FN_INSERT2 + 14)   /* insert default footer */

           (FN_INSERT2 + 18)   /* insert endnote*/
            TypedWhichId<SfxUInt16Item>(FN_INSERT2 + 19)   /* Insert section */

         (FN_INSERT2 + 20)   /* insert any TOX */
    (FN_INSERT2 + 21)   /* insert entry for table of authorities*/

// MSO legacy form fields
        (FN_INSERT2 + 22)
    (FN_INSERT2 + 23)
    (FN_INSERT2 + 24)

// MSO content controls
        (FN_INSERT2 + 25)

               (FN_INSERT2 + 26)
            (FN_INSERT2 + 27)

       (FN_INSERT2 + 28)
        (FN_INSERT2 + 29)

// clipboard table content
       (FN_INSERT2 + 30)  /* instead of the cell-by-cell copy between source and target tables */
   (FN_INSERT2 + 31)  /* paste table as new table rows */
   (FN_INSERT2 + 32)  /* paste table as new table columns */

    TypedWhichId<SfxStringItem>(FN_INSERT2 + 33 )  /* Bookmark */
 (FN_INSERT2 + 34)
 (FN_INSERT2 + 35)
 (FN_INSERT2 + 36)
 (FN_INSERT2 + 37)
 (FN_INSERT2 + 38)
 (FN_INSERT2 + 39)
 (FN_INSERT2 + 40)
 (FN_INSERT2 + 41)
 (FN_INSERT2 + 42) /* Delete content control formatting */
 (FN_INSERT2 + 43) /*insert field page count in range*/

// Region: Format
     (FN_FORMAT + 1 ) /* apply autoformat options */
      (FN_FORMAT + 2 ) /* apply autoformat during user input */
       (FN_FORMAT + 3 ) /* size */
     (FN_FORMAT + 4 ) /* size */
 (FN_FORMAT + 5 ) /* apply autoformat with Redlining */
     (FN_FORMAT + 11) /* superscript */
       (FN_FORMAT + 12) /* subscript */
       (FN_FORMAT + 13) /* small caps */

  (FN_FORMAT + 42)  /*  */
     (FN_FORMAT + 45)  /* number format in table */
        (FN_FORMAT + 48)  /* */
   (FN_FORMAT + 49)  /* columns per page */
    (FN_FORMAT + 50)  /* background */
          TypedWhichId<SfxStringItem>(FN_FORMAT + 52)  /* page */
            (FN_FORMAT + 53)  /* columns */
          (FN_FORMAT + 54)  /* initials */
         TypedWhichId<SfxStringItem>(FN_FORMAT + 56)  /* frame */
        (FN_FORMAT + 58)  /* graphic */
         TypedWhichId<SfxStringItem>(FN_FORMAT + 60)  /* table */
     (FN_FORMAT + 62)  /* area/background */

            (FN_FORMAT + 63)  /* update style */
                (FN_FORMAT + 68)  /* footnote dialog */
        (FN_FORMAT + 64)  /* Contextual footnote dialog */

             (FN_FORMAT + 72)
               (FN_FORMAT + 73)
           (FN_FORMAT + 74)

    (FN_FORMAT + 75)
   (FN_FORMAT + 76)
  (FN_FORMAT + 77)
     (FN_FORMAT + 78)
  (FN_FORMAT + 79)
  (FN_FORMAT + 80)

         (FN_FORMAT + 82)/* frame position -- " -- */

           TypedWhichId<SfxStringItem>(FN_FORMAT + 93)  /* apply page style */

     (FN_FORMAT + 98)  /* Title Page */

                TypedWhichId<SwPtrItem>(FN_FORMAT + 99)  /* TableRepresentation */
       (FN_FORMAT + 100)  /* convert text <-> table */
         (FN_FORMAT + 101)  /* */
         (FN_FORMAT + 102)  /* */
         (FN_FORMAT + 103)  /* */
         (FN_FORMAT + 104)  /* */
        (FN_FORMAT + 105)  /* */
        (FN_FORMAT + 106)  /* */
     (FN_FORMAT + 107)  /* */
      (FN_FORMAT + 108)  /* */
   TypedWhichId<SfxUInt16Item>(FN_FORMAT + 109)  /* */
           (FN_FORMAT + 110)  /* ToolBoxItem for optimizing tables */
  (FN_FORMAT + 111)  /* */
  (FN_FORMAT + 112)  /* */
         (FN_FORMAT + 113)  /* */
         (FN_FORMAT + 114)  /* */
         (FN_FORMAT + 115)  /* */
   TypedWhichId<SfxUInt16Item>(FN_FORMAT + 116)  /* */
 (FN_FORMAT + 117)  /* protect table cells */
  (FN_FORMAT + 119)  /* undo table cell protection */
    (FN_FORMAT + 120)   /* also used in SwXTextTable*/
       (FN_FORMAT + 121)  /* */

                 (FN_FORMAT + 122)  /* frame by one level up */
               (FN_FORMAT + 123)  /* frame by one level down */

     (FN_FORMAT + 128)  /* optimal cell height */
       (FN_FORMAT + 129)  /* remove current table*/
        (FN_FORMAT + 130)  /* selects the current table cell */
    (FN_FORMAT + 131)  /* convert selected text to table */
    (FN_FORMAT + 132)  /* convert a table to text */
        (FN_FORMAT + 133)  /* sorting in tables*/
        (FN_FORMAT + 134)

// Region: Page Template
 (FN_FORMAT + 130) /* left / right margin */
 (FN_FORMAT + 131) /* upper / lower margin */
 (FN_FORMAT + 135) /* */
 (FN_FORMAT + 136) /* paper tray */
 (FN_FORMAT + 137) /* */
      (FN_FORMAT + 138) /* number of columns */

/* these Ids are only required by Help and are replaced by the Ids without _DLG
 * for the dialog */
     (FN_FORMAT + 142)  /* */
     (FN_FORMAT + 143)  /* */

           (FN_FORMAT + 145)  /* Table: go to next cell */
           (FN_FORMAT + 146)  /* Table: go to previous cell */

     (FN_FORMAT + 147) /* number of repeated headlines of tables */

// Region: Ribbon
    (FN_FORMAT + 159)  /* undo cell protection for table */
// 156..158 are toolbox item ids

         (FN_FORMAT + 162)  /* frame ideal wrapping */
    (FN_FORMAT + 163)  /* toggle frame transparent wrapping */
    (FN_FORMAT + 164)  /* frame transparent wrapping */

     (FN_FORMAT + 165)  /* */
  (FN_FORMAT + 166)  /* */
  (FN_FORMAT + 167)  /* */

    (FN_FORMAT + 168)  /* */
 (FN_FORMAT + 169)  /* */
 (FN_FORMAT + 170)  /* */

              (FN_FORMAT + 172)  /* */
             (FN_FORMAT + 173)  /* */

         (FN_FORMAT + 181)  /* wrap only for first paragraph */

      (FN_FORMAT + 182)  /* evenly distribute columns */
       (FN_FORMAT + 183)  /* evenly distribute rows */

       (FN_FORMAT + 184)  /* */

// +185..+187 removed in favor of corresponding globally available slot

     TypedWhichId<SfxUInt16Item>(FN_FORMAT + 188)  /* vertical alignment in Writer table cells */
           (FN_FORMAT + 189)  /* table mode */
      (FN_FORMAT + 190)  /*  -"-  */
      (FN_FORMAT + 191)  /*  -"-  */
 TypedWhichId<SvxFrameDirectionItem>(FN_FORMAT + 192)  /* text orientation of table cells */
    TypedWhichId<SfxUInt32Item>(FN_FORMAT + 193)
  TypedWhichId<SfxUInt32Item>(FN_FORMAT2 + 194)

            (FN_FORMAT + 195)  /* */

          (FN_FORMAT2 + 9)   /* */
          (FN_FORMAT2 + 10)  /* */

           (FN_FORMAT2 + 12)  /* abstract in new document */
      (FN_FORMAT2 + 13)  /* abstract to StarImpress */

                (FN_FORMAT2 + 14)  /* Name shapes */

   (FN_FORMAT2 + 15)  /* shape title and description */

                    TypedWhichId<SfxStringItem>(FN_FORMAT2 + 120) /* set Boxes/NumberFormatter */
                  (FN_FORMAT2 + 121)
                    (FN_FORMAT2 + 123)
                (FN_FORMAT2 + 124)
                      (FN_FORMAT2 + 125)
                      (FN_FORMAT2 + 126)
                  (FN_FORMAT2 + 127)
                   (FN_FORMAT2 + 128)

                      (FN_FORMAT2 + 136)
                    (FN_FORMAT2 + 137)

                  (FN_FORMAT2 + 138)
               TypedWhichId<SfxUInt16Item>(FN_FORMAT2 + 139)

       (FN_FORMAT2 + 140)
     (FN_FORMAT2 + 141)

                (FN_FORMAT2 + 142)
                      (FN_FORMAT2 + 143)

                (FN_FORMAT2 + 144)

             (FN_FORMAT2 + 147)  /* UNO */
                      (FN_FORMAT2 + 148)  /* UNO */
          (FN_FORMAT2 + 149)  /* UNO */

                (FN_FORMAT2 + 150)
                (FN_FORMAT2 + 151)

                (FN_FORMAT2 + 152)
                  (FN_FORMAT2 + 153)

               (FN_FORMAT2 + 154)
               (FN_FORMAT2 + 155)
               (FN_FORMAT2 + 156)
             (FN_FORMAT2 + 157)
            (FN_FORMAT2 + 158)
       (FN_FORMAT2 + 159)
            (FN_FORMAT2 + 160)
           (FN_FORMAT2 + 161)
          (FN_FORMAT2 + 162)
//free (163 except 194 already used above)

// Region: Extras
       (FN_EXTRA + 2 )   /* */
        (FN_EXTRA + 5 )   /* */
                 (FN_EXTRA + 6 )   /* do not hyphenate */
              (FN_EXTRA + 7 )   /* learn words */
    (FN_EXTRA + 12)   /* */
              (FN_EXTRA + 14)   /* */
                (FN_EXTRA + 15)   /* */
             (FN_EXTRA + 20)   /* text building blocks */

          (FN_EXTRA + 28)   /* expand text building blocks */
           TypedWhichId<SfxUInt16Item>(FN_EXTRA + 34)   /* change page numbers*/
             (FN_EXTRA + 35)   /* add text box to draw shape */
          (FN_EXTRA + 36)   /* remove text box of draw shape */
           (FN_EXTRA + 37)   /* open local copy for bibliography */

//  Region: Glossary

             (FN_EXTRA + 49 )  /* Autocorrect from Basic */

               (FN_EXTRA + 53)   /* update all indices */
           (FN_EXTRA + 54)   /* update current index */
           (FN_EXTRA + 55)  /* remove the current TOX*/

                (FN_EXTRA + 98)   /* type for GlobalDoc-Collection*/
                 (FN_EXTRA + 99)

               (FN_EXTRA2 + 1)  /*  area name or index title */
             (FN_EXTRA2 + 4)  /* enable/disable Shadow Cursor */

           (FN_EXTRA2 + 9)  // starting here are UNI-Ids for the PropertyMap listed
           (FN_EXTRA2 + 10)

 (FN_EXTRA2 + 11)
     (FN_EXTRA2 + 12)
      (FN_EXTRA2 + 13)
            (FN_EXTRA2 + 14)
            (FN_EXTRA2 + 15)
  (FN_EXTRA2 + 16)
       (FN_EXTRA2 + 17)
           (FN_EXTRA2 + 18)
           (FN_EXTRA2 + 19)
                 (FN_EXTRA2 + 20)
           (FN_EXTRA2 + 21)
       (FN_EXTRA2 + 22)
         (FN_EXTRA2 + 23)
             (FN_EXTRA2 + 24)
              (FN_EXTRA2 + 25)
      (FN_EXTRA2 + 26)
      (FN_EXTRA2 + 27)
         (FN_EXTRA2 + 29)
      (FN_EXTRA2 + 30)
    (FN_EXTRA2 + 31)
        (FN_EXTRA2 + 32)
                   (FN_EXTRA2 + 33)
              (FN_EXTRA2 + 34)
                       (FN_EXTRA2 + 35)
                  (FN_EXTRA2 + 36)
                 (FN_EXTRA2 + 37)
                       (FN_EXTRA2 + 38)
                  (FN_EXTRA2 + 39)
                 (FN_EXTRA2 + 40)
               (FN_EXTRA2 + 41)
                  (FN_EXTRA2 + 42)
               (FN_EXTRA2 + 43)
                 (FN_EXTRA2 + 59)

                         TypedWhichId<SfxBoolItem>(FN_EXTRA2 + 60)

                  (FN_EXTRA2 + 61)
               (FN_EXTRA2 + 62)
                 (FN_EXTRA2 + 63)

                         (FN_EXTRA2 + 64)
                 (FN_EXTRA2 + 65)
 (FN_EXTRA2 + 66)
  (FN_EXTRA2 + 67)

                     (FN_EXTRA2 + 68)
                    (FN_EXTRA2 + 69)
                    (FN_EXTRA2 + 70)
                  (FN_EXTRA2 + 71)
                 (FN_EXTRA2 + 72)
              (FN_EXTRA2 + 73)
                      (FN_EXTRA2 + 74)
           (FN_EXTRA2 + 75)
             (FN_EXTRA2 + 76)
            (FN_EXTRA2 + 77)
            (FN_EXTRA2 + 78)
                     (FN_EXTRA2 + 79)
                 (FN_EXTRA2 + 80)
                     (FN_EXTRA2 + 81)
         (FN_EXTRA2 + 82)
             (FN_EXTRA2 + 83)
             (FN_EXTRA2 + 84)
                  (FN_EXTRA2 + 85)
                  (FN_EXTRA2 + 86)
               (FN_EXTRA2 + 87)
                    (FN_EXTRA2 + 88)
                  TypedWhichId<SfxStringItem>(FN_EXTRA2 + 89)
                     TypedWhichId<SfxStringItem>(FN_EXTRA2 + 90)
                 TypedWhichId<SfxStringItem>(FN_EXTRA2 + 91)
 (FN_EXTRA2 + 92)

             (FN_EXTRA2 + 94)
                        (FN_EXTRA2 + 95)
                        (FN_EXTRA2 + 96)
                    (FN_EXTRA2 + 97)
                 (FN_EXTRA2 + 98)

               (FN_EXTRA2 + 99)

        (FN_EXTRA2 + 100)
                      (FN_EXTRA2 + 101)

      (FN_EXTRA2 + 102)
                (FN_EXTRA2 + 103)
       (FN_EXTRA2 + 104)
             (FN_EXTRA2 + 105)
                  (FN_EXTRA2 + 106)
                      (FN_EXTRA2 + 107)
          (FN_EXTRA2 + 108)

       (FN_EXTRA2 + 109)
              (FN_EXTRA2 + 110)
                   (FN_EXTRA2 + 111)
                         (FN_EXTRA2 + 112)
          (FN_EXTRA2 + 113)

/* Navigation buttons */
                  (FN_EXTRA2 + 114)
               (FN_EXTRA2 + 115)

// #i972: bool items to be passed to SwFramePage for evaluation
                      TypedWhichId<SfxBoolItem>(FN_EXTRA2 + 116)
          TypedWhichId<SfxBoolItem>(FN_EXTRA2 + 117)

                  (FN_EXTRA2 + 118)
                  (FN_EXTRA2 + 119)
                 (FN_EXTRA2 + 120)
                 (FN_EXTRA2 + 121)
                (FN_EXTRA2 + 122)

           TypedWhichId<SfxUnoAnyItem>(FN_EXTRA2 + 123)
                       (FN_EXTRA2 + 124)
       (FN_EXTRA2 + 125)
          (FN_EXTRA2 + 126)
           (FN_EXTRA2 + 127)
          (FN_EXTRA2 + 128)

          (FN_EXTRA2 + 129)
              (FN_EXTRA2 + 130)
 (FN_EXTRA2 + 131)
 (FN_EXTRA2 + 132)
  (FN_EXTRA2 + 133)

// Area: Help
// Region: Traveling & Selection
                (FN_SELECTION + 1 ) /* */
               (FN_SELECTION + 2 ) /* */
                  (FN_SELECTION + 3 ) /* */
                (FN_SELECTION + 4 ) /* */
            (FN_SELECTION + 5 ) /* StartOfLine */
              (FN_SELECTION + 6 ) /* EndOfLine */
        (FN_SELECTION + 7 ) /* StartOfDocument */
       (FN_SELECTION + 9 ) /* StartOfNextPage ??? */
         (FN_SELECTION + 10) /* ??? */
       (FN_SELECTION + 11) /* StartOfPrevPage ??? */
         (FN_SELECTION + 12) /* ??? */
            (FN_SELECTION + 13) /* StartOfPage */
              (FN_SELECTION + 14) /* EndOfPage */
          (FN_SELECTION + 17) /* StartOfColumn */
            (FN_SELECTION + 18) /* EndOfColumn */
            (FN_SELECTION + 19) /* StartOfPara */
              (FN_SELECTION + 20) /* EndOfPara */
                (FN_SELECTION + 21) /* NextWord */
                (FN_SELECTION + 22) /* PrevWord */
                (FN_SELECTION + 23) /* NextSentence */
                (FN_SELECTION + 24) /* PrevSentence */
                   (FN_SELECTION + 25) /* Delete */
                (FN_SELECTION + 26) /* Backspace */
              (FN_SELECTION + 27) /* DeleteToEndOfSentence */
         (FN_SELECTION + 28) /* DeleteToStartOfSentence */
              (FN_SELECTION + 29) /* DeleteToEndOfWord */
         (FN_SELECTION + 30) /* DeleteToStartOfWord */
              (FN_SELECTION + 31) /* DeleteToEndOfLine */
         (FN_SELECTION + 32) /* DeleteToStartOfLine */
              (FN_SELECTION + 33) /* DeleteToEndOfPara */
         (FN_SELECTION + 34) /* DeleteToStartOfPara */
        (FN_SELECTION + 35) /* DeleteLine ??? */
                   (FN_SELECTION + 37) /* PageUpWithCursor */
                 (FN_SELECTION + 38) /* PageDownWithCursor */
             (FN_SELECTION + 39) /* Selectionmode */
             (FN_SELECTION + 40) /* Selectionmode */
                   (FN_SELECTION + 41) /* Normal */
          (FN_SELECTION + 42) /* like Backspace */
              (FN_SELECTION + 43) /* select word*/

            (FN_SELECTION + 44)   /* Goto next object */
            (FN_SELECTION + 45)   /* Go to previous object */

              (FN_SELECTION + 47) /* to the beginning of the table */
                (FN_SELECTION + 48) /* to the end of the table */
               (FN_SELECTION + 49) /* to the next table */
               (FN_SELECTION + 50) /* to the previous table */

     (FN_SELECTION + 51) /* to the start of the next column */
       (FN_SELECTION + 52) /* to the end of the next column */
     (FN_SELECTION + 53) /* to the start of the previous column */
       (FN_SELECTION + 54) /* to the end of the previous column */

       (FN_SELECTION + 55) /* from the footnote to the anchor */
            (FN_SELECTION + 56) /* to the next footnote */
            (FN_SELECTION + 57) /* to the previous footnote */

      (FN_SELECTION + 58) /* from content to the next frame */
          (FN_SELECTION + 59) /* from frame to the anchor */

                (FN_SELECTION + 60) /* toggle between content and header */
                (FN_SELECTION + 61) /* toggle between content and footer */

          (FN_SELECTION + 62) /* from the index mark to the index */

         (FN_SELECTION + 63) /* jump into the appropriate footnote area */

           (FN_SELECTION + 64) /* selection mode: block */

                (FN_SELECTION + 74)
                (FN_SELECTION + 75)

           (FN_SELECTION + 76)   /* Go to next marker */
           (FN_SELECTION + 77)   /* Go to previous marker */

             (FN_SELECTION + 83)   /* to the next index mark */
             (FN_SELECTION + 84)   /* to the previous index mark */
              (FN_SELECTION + 85)   /* to the next table formula */
              (FN_SELECTION + 86)   /* to the previous table formula */
          (FN_SELECTION + 87)   /* to the next broken table formula */
          (FN_SELECTION + 88)   /* to the previous broken table formula */

  (FN_SELECTION + 89)   /* switches text selection mode in readonly documents*/
   (FN_SELECTION + 90)   /* use default selection mode - not block mode */
     (FN_SELECTION + 91)   /* switch on block selection */
    (FN_SELECTION + 92)
       (FN_SELECTION + 93)   /* shows/hides track changes bar */

          (FN_SELECTION + 94)   /* select paragraph*/

             (FN_SELECTION + 95)
             (FN_SELECTION + 96)
          (FN_SELECTION + 97)

      (FN_SELECTION + 98)   /*Show Single Page per row*/
   (FN_SELECTION + 99)   /*Show Multiple Page per row*/
                 (FN_SELECTION + 100)   /*Show in Bookview*/

// QUERY-Block
              (FN_QUERY +29)      /* INet-Attribute */

           (FN_QUERY +32)      /* Get attribute for printable area of page */
            TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 3)
                TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 4)
           TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 5)
          TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 6)
     TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 7)
      TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 8)
        TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 9)
           TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 10)

// Region: Envelope
                  (FN_ENVELP    )

       TypedWhichId<SfxStringItem>(FN_NOTES+1)
         (FN_NOTES+2)
                (FN_NOTES+3)
         TypedWhichId<SfxStringItem>(FN_NOTES+4)
           (FN_NOTES+5)

           (FN_NOTES+6)
                    (FN_NOTES+7)
         (FN_NOTES+8)
             (FN_NOTES+9)
      (FN_NOTES+10)
    (FN_NOTES+11)
          (FN_NOTES+12)

// Region: Parameter
         TypedWhichId<SfxInt32Item>(FN_PARAM+2)
     TypedWhichId<SfxBoolItem>(FN_PARAM+3)

         TypedWhichId<SwAddPrinterItem>(FN_PARAM+18)
            TypedWhichId<SwDocDisplayItem>(FN_PARAM+20)
               TypedWhichId<SwElemItem>(FN_PARAM+21)
             TypedWhichId<SwTestItem>(FN_PARAM+22)

           TypedWhichId<SwPageFootnoteInfoItem>(FN_PARAM+23)

            TypedWhichId<SfxStringItem>(FN_PARAM+24)
       TypedWhichId<SfxStringItem>(FN_PARAM+25)
          TypedWhichId<SfxBoolItem>(FN_PARAM+26)
         TypedWhichId<SfxBoolItem>(FN_PARAM+27)
 TypedWhichId<SfxBoolItem>(FN_PARAM+28)

           TypedWhichId<SfxBoolItem>(FN_PARAM+29)

             TypedWhichId<SfxStringItem>(FN_PARAM+44)
         TypedWhichId<SfxUInt16Item>(FN_PARAM+50)

            TypedWhichId<SfxBoolItem>(FN_PARAM+53)
             TypedWhichId<SfxUInt16Item>(FN_PARAM+54)
          TypedWhichId<SfxUInt16Item>(FN_PARAM+56)
           TypedWhichId<SfxUInt32Item>(FN_PARAM+57)

                TypedWhichId<SwPtrItem>(FN_PARAM+78) /* Printer */
               TypedWhichId<SwPtrItem>(FN_PARAM+79) /* ConfigItem Standardfonts */

               TypedWhichId<SwPtrItem>(FN_PARAM2) /* SwWrtShell */

                    TypedWhichId<SwCondCollItem>(FN_PARAM2+1) /* Item for conditional templates */
              TypedWhichId<SfxStringItem>(FN_PARAM2+2) /* selected word for format/character/hyperlink */

             TypedWhichId<SwUINumRuleItem>(FN_PARAM2+3) /* PointerItem the current NumRule */

             TypedWhichId<SfxBoolItem>(FN_PARAM2+5) /* predefined numbering */

           TypedWhichId<SwShadowCursorItem>(FN_PARAM2+8) /* for ShadowCursor options */

                   (FN_PARAM2+12) /* TextRange Property*/

      TypedWhichId<SfxBoolItem>(FN_PARAM2+13) /* Cursor in protected areas */
               TypedWhichId<SfxUInt16Item>(FN_PARAM2+14) /* TOX type in tox dialog*/
      (FN_PARAM2+15) /* LinkDisplayName property*/
     TypedWhichId<SwFmtAidsAutoComplItem>(FN_PARAM2+16) /* Formatting aids autocomplete options */
             (FN_PARAM2+17) /* contour PolyPolygon*/

              (FN_PARAM2+18) /* AnchorPosition property */
                    TypedWhichId<SfxStringItem>(FN_PARAM2+18) /* text set in drop caps tab page - for recording */
         TypedWhichId<SfxStringItem>(FN_PARAM2+19) /* character style of drop caps - for recording */
         TypedWhichId<SfxStringItem>(FN_PARAM2+20) /* Name of frame to be added as chain successor */
             TypedWhichId<SfxStringItem>(FN_PARAM2+21) /* Name of frame to be added as chain predecessor */
    TypedWhichId<SfxUnoAnyItem>(FN_PARAM2+22) /* transport a Sequence<PropertyValue> containing database properties*/
 (FN_PARAM2+23)
    (FN_PARAM2+24)
 (FN_PARAM2+25)
   (FN_PARAM2+26)
                    TypedWhichId<SwPaMItem>(FN_PARAM2+27) /* Point and Mark */
                     (FN_PARAM2+28) /* TextBox Property*/
              (FN_PARAM2+29) /* Hidden Property of bookmarks*/
           (FN_PARAM2+30) /* Condition Property of bookmarks*/

// Status: not more than 19!
                TypedWhichId<SfxStringListItem>(FN_STAT + 1)
            TypedWhichId<SfxStringItem>(FN_STAT + 2)
                (FN_STAT + 3)
             TypedWhichId<SfxUInt16Item>(FN_STAT + 5)
            TypedWhichId<SfxUInt16Item>(FN_STAT + 8)  /* For Popup Bookmarks*/
           TypedWhichId<SfxStringItem>(FN_STAT + 9)
 (FN_STAT + 10)

// Region: Page preview
           (FN_PGPREVIEW + 1)
#pragma once
      (FN_PGPREVIEW + 2)
#endif
        (FN_PGPREVIEW + 3)
        (FN_PGPREVIEW + 4)
            (FN_PGPREVIEW + 5)
         (FN_PGPREVIEW + 6)

// Region: Setting Attributes
                       (FN_FRAME + 1)
                  (FN_FRAME + 2)
                     (FN_FRAME + 3)
                  (FN_FRAME + 4)
                  (FN_FRAME + 5)
                 TypedWhichId<SfxStringItem>(FN_FRAME + 6)
            TypedWhichId<SfxBoolItem>(FN_FRAME + 7)

             TypedWhichId<SfxStringItem>(FN_FRAME + 18)
                    (FN_FRAME + 19)
              TypedWhichId<SfxStringItem>(FN_FRAME + 20)
                  (FN_FRAME + 21)

            TypedWhichId<SfxInt16Item>(FN_SIDEBAR + 0)
            (FN_SIDEBAR + 3)
   TypedWhichId<SvxLongLRSpaceItem>(FN_SIDEBAR + 4)
    TypedWhichId<SvxLongULSpaceItem>(FN_SIDEBAR + 5)
     TypedWhichId<SfxInt16Item>(FN_SIDEBAR + 6)
            (FN_SIDEBAR + 7)
   TypedWhichId<SvxLongLRSpaceItem>(FN_SIDEBAR + 8)
    TypedWhichId<SvxLongULSpaceItem>(FN_SIDEBAR + 9)
     TypedWhichId<SfxInt16Item>(FN_SIDEBAR + 10)
            (FN_SIDEBAR + 11)

//Member IDs for Fill / SetVariable of items
       0xe0
      0xe1
      0xe2
       0xe3
       0xe4
       0xe5
       0xe6
        0xe7

           0xe8
           0xe9
           0xea
           0xeb
           0xec
           0xed
           0xee
           0xef
           0xf0
           0xf1
           0xf2
           0xf3
           0xf4
           0xf5
           0xf6
          0xf7
          0xf8
          0xf9
          0xfa



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
