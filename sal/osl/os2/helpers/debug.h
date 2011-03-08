/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

/*
 *@@sourcefile debug.h:
 *      header file for debug.c (exception handling and debugging).
 *      See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@changed V0.9.0: included contents of newexe.h
 *
 *@@include #define INCL_BASE
 *@@include #include <os2.h>
 *@@include #include <stdio.h>
 *@@include #include "helpers\debug.h"
 */

/*
 *      This file incorporates code from the following:
 *      -- Marc Fiammante, John Currier, Kim Rasmussen,
 *         Anthony Cruise (EXCEPT3.ZIP package for a generic
 *         exception handling DLL, available at Hobbes).
 *
 *      This file Copyright (C) 1992-99 Ulrich M�ller,
 *                                      Kim Rasmussen,
 *                                      Marc Fiammante,
 *                                      John Currier,
 *                                      Anthony Cruise.
 *
 *      2009-06-15 published under LGPL3 with Ulrich M�ller permission.
 *
 */

#ifndef DEBUG_HEADER_INCLUDED
    #define DEBUG_HEADER_INCLUDED

    /********************************************************************
     *
     *   SYM file declarations
     *
     ********************************************************************/

    // Pointer means offset from beginning of file or beginning of struct
    #pragma pack(1)

    typedef struct
    {
        unsigned short int ppNextMap;     // paragraph pointer to next map
        unsigned char      bFlags;        // symbol types
        unsigned char      bReserved1;    // reserved
        unsigned short int pSegEntry;     // segment entry point value
        unsigned short int cConsts;       // count of constants in map
        unsigned short int pConstDef;     // pointer to constant chain
        unsigned short int cSegs;         // count of segments in map
        unsigned short int ppSegDef;      // paragraph pointer to first segment
        unsigned char      cbMaxSym;      // maximum symbol-name length
        unsigned char      cbModName;     // length of module name
        char               achModName[1]; // cbModName Bytes of module-name member
    } MAPDEF;

    typedef struct
    {
        unsigned short int ppNextMap;     // always zero
        unsigned char      release;       // release number (minor version number)
        unsigned char      version;       // major version number
    } LAST_MAPDEF;

    typedef struct
    {
        unsigned short int ppNextSeg;     // paragraph pointer to next segment
        unsigned short int cSymbols;      // count of symbols in list
        unsigned short int pSymDef;       // offset of symbol chain
        unsigned short int wReserved1;    // reserved
        unsigned short int wReserved2;    // reserved
        unsigned short int wReserved3;    // reserved
        unsigned short int wReserved4;    // reserved
        unsigned char      bFlags;        // symbol types; bit 0 signals 32-bit (*UM)
        unsigned char      bReserved1;    // reserved
        unsigned short int ppLineDef;     // offset of line number record
        unsigned char      bReserved2;    // reserved
        unsigned char      bReserved3;    // reserved
        unsigned char      cbSegName;     // length of segment name
        char               achSegName[1]; /* cbSegName Bytes of segment-name member*/
    } SEGDEF;

    typedef struct
    {
        unsigned short int wSymVal;       // symbol address or constant
        unsigned char      cbSymName;     // length of symbol name
        char               achSymName[1]; // cbSymName Bytes of symbol-name member
    } SYMDEF16;

    typedef struct
    {
        unsigned       int wSymVal;       // symbol address or constant
        unsigned char      cbSymName;     // length of symbol name
        char               achSymName[1]; // cbSymName Bytes of symbol-name member
    } SYMDEF32;

    typedef struct
    {
        unsigned short int ppNextLine;    // ptr to next linedef (0 if last)
        unsigned short int wReserved1;    // reserved
        unsigned short int pLines;        // pointer to line numbers
        unsigned short int cLines;        // reserved
        unsigned char      cbFileName;    // length of filename
        char               achFileName[1];// cbFileName Bytes of filename
    } LINEDEF;

    typedef struct
    {
        unsigned short int wCodeOffset;   // executable offset
        unsigned short int dwFileOffset;  // source offset
    } LINEINF;

    #define SEGDEFOFFSET(MapDef)     (MapDef.ppSegDef*16)
    #define NEXTSEGDEFOFFSET(SegDef)  (SegDef.ppNextSeg*16)

    #define ASYMPTROFFSET(SegDefOffset,Segdef) (SegDefOffset+SegDef.pSymDef)
    #define SYMDEFOFFSET(SegDefOffset,SegDef,n) (ASYMPTROFFSET(SegDefOffset,SegDef)+(n)*(sizeof(unsigned short int)))

    #define ACONSTPTROFFSET(MapDef) (MapDef.ppConstDef)
    #define CONSTDEFOFFSET(MapDef,n) ((MapDef.ppConstDef)+(n)*(sizeof(unsigned short int)))

    #define LINEDEFOFFSET(SegDef) (SegDef.ppLineDef*16)
    #define NEXTLINEDEFOFFSET(LineDef) (LineDef.ppNextLine*16)
    #define LINESOFFSET(LinedefOffset,LineDef) ((LinedefOffset)+LineDef.pLines)

    /********************************************************************
     *
     *   EXE declarations taken from exe.h
     *
     ********************************************************************/

    #ifndef WORD
    typedef unsigned short WORD;
    #endif

    typedef WORD bbmask;

    struct exe {
            WORD eid;                       // contains EXEID, below
            WORD elast;                     // # of bytes in last page
            WORD epagsiz;                   // # of pages in whole file
            WORD erelcnt;                   // # of relocation entrys
            WORD ehdrsiz;                   // size of header, in paragraphs
            WORD eminfre;                   // min # of free paragraphs needed
            WORD emaxfre;                   // max # of free paragraphs needed
            WORD eiSS;                      // initial SS value
            WORD eiSP;                      // initial SP value
            WORD enegsum;                   // negative sum of entire file
            WORD eiIP;                      // initial IP value
            WORD eiCS;                      // initial CS value
            WORD ereloff;                   // offset in file of relocation table
            WORD eovlnum;                   // number of the overlay

    /*      the following fields may not be present.
     *              ereloff = 28            not present
     *                      = 30            exe.ever present and valid
     *                      = 32            exe.ever field contains garbage
     *              ereloff > 32            exe.ever present and valid
     *                                              = 0 if "don't know"
     */

            WORD ever;                      // version # of producing linker
            WORD dumy;                      // unused

    /*      the following fields may not be present - if the exe.ereloff
     *      value encompasses the fields then they are present and valid.
     */

            bbmask  ebb;                    // behavior bits
            WORD dumy2[7];                  // must be 0 until defined
    };


    #define EXEID 0x5a4d                    // magic ID value

    /********************************************************************
     *
     *   Object Module Format (OMF) declarations
     *
     ********************************************************************/

    struct exehdr_rec
    {
       BYTE     signature[2];              // Must be "MZ"
       USHORT   image_len;                 // Image Length
       USHORT   pages;                     // Pages
       USHORT   reloc_items;               // Relocation table items
       USHORT   min_paragraphs;            // Mininum 16-bytes paragraphs
       USHORT   max_paragraphs;            // Maximum 16-bytes paragraphs
       USHORT   stack_pos;                 // Stack position
       USHORT   offset_in_sp;              // Offset in SP
       USHORT   checksum;                  // Checksum
       USHORT   offset_in_ip;              // Offset in IP
       USHORT   code_pos;                  // Code segment pos.
       USHORT   reloc_item_pos;            // Position of first relocation item
       USHORT   overlay_number;            // Overlay number
       BYTE     unused[8];                 // Unused bytes
       USHORT   oem_id;                    // OEM Identifier
       BYTE     oem_info[24];              // OEM Info
       ULONG    lexe_offset;               // Offset to linear header
    };

    struct lexehdr_rec
    {
       BYTE     signature[2];              // Must be "LX"
       BYTE     b_ord;                     // Byte ordering
       BYTE     w_ord;                     // Word ordering
       ULONG    format_level;              // Format level
       USHORT   cpu_type;                  // CPU Type
       USHORT   os_type;                   // Operating system
       ULONG    module_version;            // Module version
       ULONG    mod_flags;                 // Module flags
       ULONG    mod_pages;                 // Module pages
       ULONG    EIP_object;                // EIP Object no.
       ULONG    EIP;                       // EIP Value
       ULONG    ESP_object;                // ESP Object no
       ULONG    ESP;                       // ESP Value
       ULONG    page_size;                 // Page size
       ULONG    page_ofs_shift;            // Page offset shift
       ULONG    fixup_sect_size;           // Fixup section size
       ULONG    fixup_sect_checksum;       // Fixup section checksum
       ULONG    loader_sect_size;          // Loader section size
       ULONG    loader_sect_checksum;      // Loader section checksum
       ULONG    obj_table_ofs;             // Object table offset
       ULONG    obj_count;                 // Object count
       ULONG    obj_page_tab_ofs;          // Object page table offset
       ULONG    obj_iter_page_ofs;         // Object iteration pages offset
       ULONG    res_tab_ofs;               // Resource table offset
       ULONG    res_table_entries;         // Resource table entries
       ULONG    res_name_tab_ofs;          // Resident name table offset;
       ULONG    ent_tab_ofs;               // Entry table offset
       ULONG    mod_dir_ofs;               // Module directives offset
       ULONG    mod_dir_count;             // Number of module directives
       ULONG    fixup_page_tab_ofs;        // Fixup page table offset
       ULONG    fixup_rec_tab_ofs;         // Fixup record table offset
       ULONG    imp_tab_ofs;               // Import module table offset
       ULONG    imp_mod_entries;           // Import module entries
       ULONG    imp_proc_tab_ofs;          // Import proc table offset
       ULONG    per_page_check_ofs;        // Per page checksum offset
       ULONG    data_page_offset;          // Data pages offset
       ULONG    preload_page_count;        // Preload pages count
       ULONG    nonres_tab_ofs;            // Nonresident name table offset
       ULONG    nonres_tab_len;            // Nonresident name table len
       ULONG    nonres_tab_check;          // Nonresident tables checksum
       ULONG    auto_ds_objectno;          // Auto DS object number
       ULONG    debug_info_ofs;            // Debug info offset
       ULONG    debug_info_len;            // Debug info length
       ULONG    inst_preload_count;        // Instance preload count
       ULONG    inst_demand_count;         // Instance demand count
       ULONG    heapsize;                  // Heap size
       ULONG    stacksize;                 // Stack size
    };

    struct debug_head_rec
    {
       BYTE signature[3];                  // Debug signature
       BYTE type;                          // Debug info type
    };

    struct dir_inf_rec
    {
       USHORT   dirstruct_size;            // Size of directory structure
       USHORT   number_of_entries;         // Number of dnt_rec's in the array
       USHORT   unknown;                   // Unknown data
       // Followed by an array of dnt_rec structures
    };

    struct dnt_rec
    {
       USHORT   subsect_type;              // sst Subsection type
       USHORT   mod_index;                 // Module index (1-based)
       ULONG    offset;                    // Offset of start of section
       ULONG    size;                      // Size of section
    };

    // Modules subsection
    struct modules_rec
    {
       USHORT   code_seg_base;             // Code segment base
       ULONG    code_seg_offset;           // Code segment offset
       ULONG    code_seg_len;              // Code segment length
       USHORT   overlay_no;                // Overlay number
       USHORT   lib_idx;                   // Index into library section or 0
       BYTE     segments;                  // Number of segments
       BYTE     reserved;
       BYTE     debug_style[2];            // "HL" for HLL, "CV" or 0 for CodeView
       BYTE     debug_version[2];          // 00 01 or 00 03 for HLL, 00 00 for CV
       BYTE     name_len;                  // Length of name (which follows)
    };

    // Publics subsection
    struct publics_rec
    {
       ULONG    offset;                    // Offset
       USHORT   segment;                   // Segment
       USHORT   type;                      // Type index
       BYTE     name_len;                  // Length of name (wich follows)
    };

    // First linenumber record
    struct linfirst_rec
    {
       USHORT   lineno;                    // Line number (0)
       BYTE     entry_type;                // Entry type
       BYTE     reserved;                  // Reserved
       USHORT   entries_count;             // Number of table entries
       USHORT   segment_no;                // Segment number
       ULONG    filename_tabsize;          // File names table size
    };

    // Source line numbers
    struct linsource_rec
    {
       USHORT   source_line;               // Source file line number
       USHORT   source_idx;                // Source file index
       ULONG    offset;                    // Offset into segment
    };

    // Listing statement numbers
    struct linlist_rec
    {
       ULONG    list_line;                 // Listing file linenumber
       ULONG    statement;                 // Listing file statement number
       ULONG    offset;                    // Offset into segment
    };

    // Source and Listing statement numbers
    struct linsourcelist_rec
    {
       USHORT   source_line;               // Source file line number
       USHORT   source_idx;                // Source file index
       ULONG    list_line;                 // Listing file linenumber
       ULONG    statement;                 // Listing file statement number
       ULONG    offset;                    // Offset into segment
    };

    // Path table
    struct pathtab_rec
    {
       ULONG    offset;                    // Offset into segment
       USHORT   path_code;                 // Path code
       USHORT   source_idx;                // Source file index
    };

    // File names table
    struct filenam_rec
    {
       ULONG    first_char;                // First displayable char in list file
       ULONG    disp_chars;                // Number of displayable chars in list line
       ULONG    filecount;                 // Number of source/listing files
    };

    // Symbol types
    #define SYM_BEGIN          0x00        // Begin block
    #define SYM_PROC           0x01        // Function
    #define SYM_END            0x02        // End block of function
    #define SYM_AUTO           0x04        // Auto variable
    #define SYM_STATIC         0x05        // Static variable
    #define SYM_LABEL          0x0B        // Label
    #define SYM_WITH           0x0C        // With start symbol (not used)
    #define SYM_REG            0x0D        // Register variable
    #define SYM_CONST          0x0E        // Constant
    #define SYM_ENTRY          0x0F        // Secondary entry (not in C)
    #define SYM_SKIP           0x10        // For incremental linking (not used)
    #define SYM_CHANGESEG      0x11        // Change segment (#pragma alloc_text)
    #define SYM_TYPEDEF        0x12        // Typedef variable
    #define SYM_PUBLIC         0x13        // Public reference
    #define SYM_MEMBER         0x14        // Member of minor or major structure
    #define SYM_BASED          0x15        // Based variable
    #define SYM_TAG            0x16        // Tag in struct, union, enum ...
    #define SYM_TABLE          0x17        // Table (used in RPG - not C)
    #define SYM_MAP            0x18        // Map variable (extern in C)
    #define SYM_CLASS          0x19        // Class symbol (C++)
    #define SYM_MEMFUNC        0x1A        // Member function
    #define SYM_AUTOSCOPE      0x1B        // Scoped auto for C++ (not used)
    #define SYM_STATICSCOPE    0x1C        // scoped static for C++ (not used)
    #define SYM_CPPPROC        0x1D        // C++ Proc
    #define SYM_CPPSTAT        0x1E        // C++ Static var
    #define SYM_COMP           0x40        // Compiler information

    // Symbolic begin record
    struct symbegin_rec
    {
       ULONG    offset;                    // Segment offset
       ULONG    length;                    // Length of block
       BYTE     name_len;                  // Length of block name
       // Block name follows
    };

    // Symbolic auto var record
    struct symauto_rec
    {
       ULONG    stack_offset;              // Stack offset
       USHORT   type_idx;                  // Type index
       BYTE     name_len;                  // Length of name
       // Var name follows
    };

    // Symbolic procedure record
    struct symproc_rec
    {
       ULONG    offset;                    // Segment offset
       USHORT   type_idx;                  // Type index
       ULONG    length;                    // Length of procedure
       USHORT   pro_len;                   // Length of prologue
       ULONG    pro_bodylen;               // Length of prologue + body
       USHORT   class_type;                // Class type
       BYTE     near_far;                  // Near or far
       BYTE     name_len;                  // Length of name
       // Function name follows
    };

    // Symbolic static var record
    struct symstatic_rec
    {
       ULONG    offset;                    // Segment offset
       USHORT   segaddr;                   // Segment address
       USHORT   type_idx;                  // Type index
       BYTE     name_len;                  // Length of name
       // Var name follows
    };

    // Symbolic label var record
    struct symlabel_rec
    {
       ULONG    offset;                    // Segment offset
       BYTE     near_far;                  // Near or far
       BYTE     name_len;                  // Length of name
       // Var name follows
    };

    // Symbolic register var record
    struct symreg_rec
    {
       USHORT   type_idx;                  // Type index
       BYTE     reg_no;                    // Register number
       BYTE     name_len;                  // Length of name
       // Var name follows
    };

    // Symbolic change-segment record
    struct symseg_rec
    {
       USHORT   seg_no;                    // Segment number
    };

    // Symbolic typedef record
    struct symtypedef_rec
    {
       USHORT   type_idx;                  // Type index
       BYTE     name_len;                  // Length of name
       // Name follows
    };

    // Symbolic public record
    struct sympublic_rec
    {
       ULONG    offset;                    // Segment offset
       USHORT   segaddr;                   // Segment address
       USHORT   type_idx;                  // Type index
       BYTE     name_len;                  // Length of name
       // Name follows
    };

    // Symbolic member record
    struct symmember_rec
    {
       ULONG    offset;                    // Offset to subrecord
       BYTE     name_len;                  // Length of name
       // Name follows
    };

    // Symbolic based record
    struct symbased_rec
    {
       ULONG    offset;                    // Offset to subrecord
       USHORT   type_idx;                  // Type index
       BYTE     name_len;                  // Length of name
       // Name follows
    };

    // Symbolic tag record
    struct symtag_rec
    {
       USHORT   type_idx;                  // Type index
       BYTE     name_len;                  // Length of name
       // Name follows
    };

    // Symbolic table record
    struct symtable_rec
    {
       ULONG    offset;                    // Segment offset
       USHORT   segaddr;                   // Segment address
       USHORT   type_idx;                  // Type index
       ULONG    idx_ofs;                   // Index offset to subrecord
       BYTE     name_len;                  // Length of name
       // Name follows
    };

    // Type record
    struct type_rec
    {
       USHORT   length;                    // Length of sub-record
       BYTE     type;                      // Sub-record type
       BYTE     type_qual;                 // Type qualifier
    };

    // Types
    #define TYPE_CLASS         0x40        // Class
    #define TYPE_BASECLASS     0x41        // Base class
    #define TYPE_FRIEND        0x42        // Friend
    #define TYPE_CLASSDEF      0x43        // Class definition
    #define TYPE_MEMBERFUNC    0x45        // Member function
    #define TYPE_CLASSMEMBER   0x46        // Class member
    #define TYPE_REF           0x48        // Reference
    #define TYPE_MEMBERPTR     0x49        // Member pointer
    #define TYPE_SCALARS       0x51        // Scalars
    #define TYPE_SET           0x52        // Set
    #define TYPE_ENTRY         0x53        // Entry
    #define TYPE_FUNCTION      0x54        // Function
    #define TYPE_AREA          0x55        // Area
    #define TYPE_LOGICAL       0x56        // Logical
    #define TYPE_STACK         0x57        // Stack
    #define TYPE_MACRO         0x59        // Macro
    #define TYPE_BITSTRING     0x5C        // Bit string
    #define TYPE_USERDEF       0x5D        // User defined
    #define TYPE_CHARSTR       0x60        // Character string
    #define TYPE_PICTURE       0x61        // Picture
    #define TYPE_GRAPHIC       0x62        // Graphic
    #define TYPE_FORMATLAB     0x65        // Format label
    #define TYPE_FILE          0x67        // File
    #define TYPE_SUBRANGE      0x6F        // Subrange
    #define TYPE_CODELABEL     0x72        // Code label
    #define TYPE_PROCEDURE     0x75        // Procedure
    #define TYPE_ARRAY         0x78        // Array
    #define TYPE_STRUCT        0x79        // Structure / Union / Record
    #define TYPE_POINTER       0x7A        // Pointer
    #define TYPE_ENUM          0x7B        // Enum
    #define TYPE_LIST          0x7F        // List

    // Type userdef
    struct type_userdefrec
    {
       BYTE     FID_index;                 // Field ID
       USHORT   type_index;                // Type index
       BYTE     FID_string;                // String ID
       BYTE     name_len;                  // Length of name which follows
    };

    // Type function
    struct type_funcrec
    {
       USHORT   params;
       USHORT   max_params;
       BYTE     FID_index;                 // Field ID
       USHORT   type_index;                // Type index of return value
       BYTE     FID_index1;                // String ID
       USHORT   typelist_index;            // Index of list of params
    };

    // Type struct
    struct type_structrec
    {
       ULONG    size;                      // Size of structure
       USHORT   field_count;               // Number of fields in structure
       BYTE     FID_index;                 // Field ID
       USHORT   type_list_idx;             // Index to type list
       BYTE     FID_index1;                // Field ID
       USHORT   type_name_idx;             // Index to names / offsets
       BYTE     dont_know;                 // Haven't a clue, but it seems to be needed
       BYTE     name_len;                  // Length of structure name which follows
    };

    // Type list, type qualifier 1: contains types for structures
    // This record is repeated for the number of items in the structure definition
    struct type_list1
    {
       BYTE     FID_index;                 // Field identifier for index
       USHORT   type_index;                // Type index.
    };

    // Type list, type qualifier 2: contains names and offsets for structure items
    // This record is repeated for the number of items in the structure definition
    struct type_list2
    {
       BYTE     FID_string;                // String identifier
       BYTE     name_len;                  // Length of name which follows
    };

    // Type list, subrecord to the above, contains offset of variable in the structure
    struct type_list2_1
    {
       BYTE     FID_span;                  // Defines what type of variable follows
       union {
          BYTE   b_len;
          USHORT s_len;
          ULONG  l_len;
       } u;
    };

    // Type pointer
    struct type_pointerrec
    {
       BYTE     FID_index;                 // Index identifier
       USHORT   type_index;                // Type index
       BYTE     FID_string;                // String identifier
       BYTE     name_len;                  // Length of name which follows
    };

    /********************************************************************
     *
     *   Prototypes
     *
     ********************************************************************/

    BOOL dbgPrintStackFrame(FILE *LogFile,
                            PSZ pszModuleName,  // in: module name (fully q'fied)
                            ULONG ulObject,
                            ULONG ulOffset);

    VOID dbgPrintStack(FILE *file,
                       PUSHORT StackBottom,
                       PUSHORT StackTop,
                       PUSHORT Ebp,
                       PUSHORT ExceptionAddress);

    APIRET APIENTRY DosQueryModFromEIP(HMODULE *phMod,   // out: trapping module
                                       ULONG *pulObjNum, // out: object/segment number
                                       ULONG ulBuffLen,  // in:  sizeof(*pszBuff)
                                       CHAR *pszBuff,    // out: module name
                                       ULONG *pulOffset, // out: offset in module
                                       ULONG ulAddress); // in:  address to be analyzed

    APIRET APIENTRY DOSQUERYMODFROMEIP(HMODULE * phMod,
                                       ULONG * pObjNum,
                                       ULONG BuffLen,
                                       PCHAR pBuff,
                                       ULONG * pOffset,
                                       PVOID Address);

    typedef ULONG  *_Seg16 PULONG16;
    APIRET16 APIENTRY16 DOS16SIZESEG(USHORT Seg, PULONG16 Size);
    typedef         APIRET16(APIENTRY16 _PFN16) (VOID);
    ULONG APIENTRY  DosSelToFlat(ULONG);

    APIRET16 APIENTRY16 DOSQPROCSTATUS(ULONG * _Seg16 pBuf, USHORT cbBuf);

    #define CONVERT(fp,QSsel) MAKEP((QSsel),OFFSETOF(fp))

    #pragma pack()              // added V0.9.0


    /********************************************************************
     *
     *   Executable definitions
     *
     ********************************************************************/

    #define EXE386 1

    #ifndef __NEWEXE__
        #define __NEWEXE__

        #pragma pack(1)

        /*_________________________________________________________________*
         |                                                                 |
         |                                                                 |
         |  DOS3 .EXE FILE HEADER DEFINITION                               |
         |                                                                 |
         |_________________________________________________________________|
         *                                                                 */


        #define EMAGIC          0x5A4D          // Old magic number
        #define ENEWEXE         sizeof(struct exe_hdr)
                                                // Value of E_LFARLC for new .EXEs
        #define ENEWHDR         0x003C          // Offset in old hdr. of ptr. to new
        #define ERESWDS         0x0010          // No. of reserved words (OLD)
        #define ERES1WDS        0x0004          // No. of reserved words in e_res
        #define ERES2WDS        0x000A          // No. of reserved words in e_res2
        #define ECP             0x0004          // Offset in struct of E_CP
        #define ECBLP           0x0002          // Offset in struct of E_CBLP
        #define EMINALLOC       0x000A          // Offset in struct of E_MINALLOC

        /*
         *@@ exe_hdr:
         *      DOS 1, 2, 3 .EXE header.
         */

        struct exe_hdr
        {
            unsigned short      e_magic;        // Magic number
            unsigned short      e_cblp;         // Bytes on last page of file
            unsigned short      e_cp;           // Pages in file
            unsigned short      e_crlc;         // Relocations
            unsigned short      e_cparhdr;      // Size of header in paragraphs
            unsigned short      e_minalloc;     // Minimum extra paragraphs needed
            unsigned short      e_maxalloc;     // Maximum extra paragraphs needed
            unsigned short      e_ss;           // Initial (relative) SS value
            unsigned short      e_sp;           // Initial SP value
            unsigned short      e_csum;         // Checksum
            unsigned short      e_ip;           // Initial IP value
            unsigned short      e_cs;           // Initial (relative) CS value
            unsigned short      e_lfarlc;       // File address of relocation table
            unsigned short      e_ovno;         // Overlay number
            unsigned short      e_res[ERES1WDS];// Reserved words
            unsigned short      e_oemid;        // OEM identifier (for e_oeminfo)
            unsigned short      e_oeminfo;      // OEM information; e_oemid specific
            unsigned short      e_res2[ERES2WDS];// Reserved words
            long                e_lfanew;       // File address of new exe header
        };

        #define E_MAGIC(x)      (x).e_magic
        #define E_CBLP(x)       (x).e_cblp
        #define E_CP(x)         (x).e_cp
        #define E_CRLC(x)       (x).e_crlc
        #define E_CPARHDR(x)    (x).e_cparhdr
        #define E_MINALLOC(x)   (x).e_minalloc
        #define E_MAXALLOC(x)   (x).e_maxalloc
        #define E_SS(x)         (x).e_ss
        #define E_SP(x)         (x).e_sp
        #define E_CSUM(x)       (x).e_csum
        #define E_IP(x)         (x).e_ip
        #define E_CS(x)         (x).e_cs
        #define E_LFARLC(x)     (x).e_lfarlc
        #define E_OVNO(x)       (x).e_ovno
        #define E_RES(x)        (x).e_res
        #define E_OEMID(x)      (x).e_oemid
        #define E_OEMINFO(x)    (x).e_oeminfo
        #define E_RES2(x)       (x).e_res2
        #define E_LFANEW(x)     (x).e_lfanew


            /*_________________________________________________________________*
             |                                                                 |
             |                                                                 |
             |  OS/2 & WINDOWS .EXE FILE HEADER DEFINITION - 286 version       |
             |                                                                 |
             |_________________________________________________________________|
             *                                                                 */

        #define NEMAGIC         0x454E          // New magic number
        #define NERESBYTES      8               // Eight bytes reserved (now)
        #define NECRC           8               // Offset into new header of NE_CRC

        /*
         *@@ new_exe:
         *      New Executable (NE) header.
         *      Follows DOS header in the executable file.
         */

        struct new_exe
        {
            unsigned short      ne_magic;       // Magic number NE_MAGIC
            unsigned char       ne_ver;         // Version number
            unsigned char       ne_rev;         // Revision number
            unsigned short      ne_enttab;      // Offset of Entry Table
            unsigned short      ne_cbenttab;    // Number of bytes in Entry Table
            long                ne_crc;         // Checksum of whole file
            unsigned short      ne_flags;       // Flag word
            unsigned short      ne_autodata;    // Automatic data segment number
            unsigned short      ne_heap;        // Initial heap allocation
            unsigned short      ne_stack;       // Initial stack allocation
            long                ne_csip;        // Initial CS:IP setting
            long                ne_sssp;        // Initial SS:SP setting
            unsigned short      ne_cseg;        // Count of file segments
            unsigned short      ne_cmod;        // Entries in Module Reference Table
            unsigned short      ne_cbnrestab;   // Size of non-resident name table
            unsigned short      ne_segtab;      // Offset of Segment Table
            unsigned short      ne_rsrctab;     // Offset of Resource Table
            unsigned short      ne_restab;      // Offset of resident name table
            unsigned short      ne_modtab;      // Offset of Module Reference Table
            unsigned short      ne_imptab;      // Offset of Imported Names Table
            long                ne_nrestab;     // Offset of Non-resident Names Table
            unsigned short      ne_cmovent;     // Count of movable entries
            unsigned short      ne_align;       // Segment alignment shift count
            unsigned short      ne_cres;        // Count of resource entries
            unsigned char       ne_exetyp;      // Target operating system
            unsigned char       ne_flagsothers; // Other .EXE flags
            char                ne_res[NERESBYTES]; // Pad structure to 64 bytes
        };

        #pragma pack()

        #define NE_MAGIC(x)         (x).ne_magic
        #define NE_VER(x)           (x).ne_ver
        #define NE_REV(x)           (x).ne_rev
        #define NE_ENTTAB(x)        (x).ne_enttab
        #define NE_CBENTTAB(x)      (x).ne_cbenttab
        #define NE_CRC(x)           (x).ne_crc
        #define NE_FLAGS(x)         (x).ne_flags
        #define NE_AUTODATA(x)      (x).ne_autodata
        #define NE_HEAP(x)          (x).ne_heap
        #define NE_STACK(x)         (x).ne_stack
        #define NE_CSIP(x)          (x).ne_csip
        #define NE_SSSP(x)          (x).ne_sssp
        #define NE_CSEG(x)          (x).ne_cseg
        #define NE_CMOD(x)          (x).ne_cmod
        #define NE_CBNRESTAB(x)     (x).ne_cbnrestab
        #define NE_SEGTAB(x)        (x).ne_segtab
        #define NE_RSRCTAB(x)       (x).ne_rsrctab
        #define NE_RESTAB(x)        (x).ne_restab
        #define NE_MODTAB(x)        (x).ne_modtab
        #define NE_IMPTAB(x)        (x).ne_imptab
        #define NE_NRESTAB(x)       (x).ne_nrestab
        #define NE_CMOVENT(x)       (x).ne_cmovent
        #define NE_ALIGN(x)         (x).ne_align
        #define NE_CRES(x)          (x).ne_cres
        #define NE_RES(x)           (x).ne_res
        #define NE_EXETYP(x)        (x).ne_exetyp
        #define NE_FLAGSOTHERS(x)   (x).ne_flagsothers

        #define NE_USAGE(x)     (WORD)*((WORD *)(x)+1)
        #define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
        #define NE_ONEWEXE(x)   (WORD)(x).ne_crc
        #define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)


        /*
         *  Target operating systems
         */

        #define NE_UNKNOWN      0x0             /* Unknown (any "new-format" OS) */
        #define NE_OS2          0x1             /* OS/2 (default)  */
        #define NE_WINDOWS      0x2             /* Windows */
        #define NE_DOS4         0x3             /* DOS 4.x */
        #define NE_DEV386       0x4             /* Windows 386 */


        /*
         *  Format of NE_FLAGS(x):
         *
         *  p                                   Not-a-process
         *   x                                  Unused
         *    e                                 Errors in image
         *     x                                Unused
         *      b                               Bound Family/API
         *       ttt                            Application type
         *          f                           Floating-point instructions
         *           3                          386 instructions
         *            2                         286 instructions
         *             0                        8086 instructions
         *              P                       Protected mode only
         *               p                      Per-process library initialization
         *                i                     Instance data
         *                 s                    Solo data
         */
        #define NENOTP          0x8000          /* Not a process */
        #define NEIERR          0x2000          /* Errors in image */
        #define NEBOUND         0x0800          /* Bound Family/API */
        #define NEAPPTYP        0x0700          /* Application type mask */
        #define NENOTWINCOMPAT  0x0100          /* Not compatible with P.M. Windowing */
        #define NEWINCOMPAT     0x0200          /* Compatible with P.M. Windowing */
        #define NEWINAPI        0x0300          /* Uses P.M. Windowing API */
        #define NEFLTP          0x0080          /* Floating-point instructions */
        #define NEI386          0x0040          /* 386 instructions */
        #define NEI286          0x0020          /* 286 instructions */
        #define NEI086          0x0010          /* 8086 instructions */
        #define NEPROT          0x0008          /* Runs in protected mode only */
        #define NEPPLI          0x0004          /* Per-Process Library Initialization */
        #define NEINST          0x0002          /* Instance data */
        #define NESOLO          0x0001          /* Solo data */

        /*
         *  Format of NE_FLAGSOTHERS(x):
         *
         *      7 6 5 4 3 2 1 0  - bit no
         *      |         | | |
         *      |         | | +---------------- Support for long file names
         *      |         | +------------------ Windows 2.x app runs in prot mode
         *      |         +-------------------- Windows 2.x app gets prop. font
         *      +------------------------------ WLO appl on OS/2 (markwlo.exe)
         *
         */

        #define NELONGNAMES     0x01
        #define NEWINISPROT     0x02
        #define NEWINGETPROPFON 0x04
        #define NEWLOAPPL       0x80



        struct new_seg                          /* New .EXE segment table entry */
          {
            unsigned short      ns_sector;      /* File sector of start of segment */
            unsigned short      ns_cbseg;       /* Number of bytes in file */
            unsigned short      ns_flags;       /* Attribute flags */
            unsigned short      ns_minalloc;    /* Minimum allocation in bytes */
          };

        #define NS_SECTOR(x)    (x).ns_sector
        #define NS_CBSEG(x)     (x).ns_cbseg
        #define NS_FLAGS(x)     (x).ns_flags
        #define NS_MINALLOC(x)  (x).ns_minalloc


        /*
         *  Format of NS_FLAGS(x)
         *
         *  Flag word has the following format:
         *
         *      15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
         *          |  |  |  |  | | | | | | | | | | |
         *          |  |  |  |  | | | | | | | | +-+-+--- Segment type DATA/CODE
         *          |  |  |  |  | | | | | | | +--------- Iterated segment
         *          |  |  |  |  | | | | | | +----------- Movable segment
         *          |  |  |  |  | | | | | +------------- Segment can be shared
         *          |  |  |  |  | | | | +--------------- Preload segment
         *          |  |  |  |  | | | +----------------- Execute/read-only for code/data segment
         *          |  |  |  |  | | +------------------- Segment has relocations
         *          |  |  |  |  | +--------------------- Code conforming/Data is expand down
         *          |  |  |  +--+----------------------- I/O privilege level
         *          |  |  +----------------------------- Discardable segment
         *          |  +-------------------------------- 32-bit code segment
         *          +----------------------------------- Huge segment/GDT allocation requested
         *
         */

        #define NSTYPE          0x0007          /* Segment type mask */

        #ifdef EXE386
            #define NSCODE          0x0000          /* Code segment */
            #define NSDATA          0x0001          /* Data segment */
            #define NSITER          0x0008          /* Iterated segment flag */
            #define NSMOVE          0x0010          /* Movable segment flag */
            #define NSSHARED        0x0020          /* Shared segment flag */
            #define NSPRELOAD       0x0040          /* Preload segment flag */
            #define NSEXRD          0x0080          /* Execute-only (code segment), or
                                                    *  read-only (data segment)
                                                    */
            #define NSRELOC         0x0100          /* Segment has relocations */
            #define NSCONFORM       0x0200          /* Conforming segment */
            #define NSEXPDOWN       0x0200          /* Data segment is expand down */
            #define NSDPL           0x0C00          /* I/O privilege level (286 DPL bits) */
            #define SHIFTDPL        10              /* Left shift count for SEGDPL field */
            #define NSDISCARD       0x1000          /* Segment is discardable */
            #define NS32BIT         0x2000          /* 32-bit code segment */
            #define NSHUGE          0x4000          /* Huge memory segment, length of
                                                     * segment and minimum allocation
                                                     * size are in segment sector units
                                                     */
            #define NSGDT           0x8000          /* GDT allocation requested */

            #define NSPURE          NSSHARED        /* For compatibility */

            #define NSALIGN 9       /* Segment data aligned on 512 byte boundaries */

            #define NSLOADED    0x0004      /* ns_sector field contains memory addr */
        #endif


        struct new_segdata                      /* Segment data */
          {
            union
              {
                struct
                  {
                    unsigned short      ns_niter;       /* number of iterations */
                    unsigned short      ns_nbytes;      /* number of bytes */
                    char                ns_iterdata;    /* iterated data bytes */
                  } ns_iter;
                struct
                  {
                    char                ns_data;        /* data bytes */
                  } ns_noniter;
              } ns_union;
          };

        struct new_rlcinfo                      /* Relocation info */
          {
            unsigned short      nr_nreloc;      /* number of relocation items that */
          };                                    /* follow */

        #pragma pack(1)


        struct new_rlc                          /* Relocation item */
          {
            char                nr_stype;       /* Source type */
            char                nr_flags;       /* Flag byte */
            unsigned short      nr_soff;        /* Source offset */
            union
              {
                struct
                  {
                    char        nr_segno;       /* Target segment number */
                    char        nr_res;         /* Reserved */
                    unsigned short nr_entry;    /* Target Entry Table offset */
                  }             nr_intref;      /* Internal reference */
                struct
                  {
                    unsigned short nr_mod;      /* Index into Module Reference Table */
                    unsigned short nr_proc;     /* Procedure ordinal or name offset */
                  }             nr_import;      /* Import */
                struct
                  {
                    unsigned short nr_ostype;   /* OSFIXUP type */
                    unsigned short nr_osres;    /* reserved */
                  }             nr_osfix;       /* Operating system fixup */
              }                 nr_union;       /* Union */
          };

        #pragma pack()


        #define NR_STYPE(x)     (x).nr_stype
        #define NR_FLAGS(x)     (x).nr_flags
        #define NR_SOFF(x)      (x).nr_soff
        #define NR_SEGNO(x)     (x).nr_union.nr_intref.nr_segno
        #define NR_RES(x)       (x).nr_union.nr_intref.nr_res
        #define NR_ENTRY(x)     (x).nr_union.nr_intref.nr_entry
        #define NR_MOD(x)       (x).nr_union.nr_import.nr_mod
        #define NR_PROC(x)      (x).nr_union.nr_import.nr_proc
        #define NR_OSTYPE(x)    (x).nr_union.nr_osfix.nr_ostype
        #define NR_OSRES(x)     (x).nr_union.nr_osfix.nr_osres



        /*
         *  Format of NR_STYPE(x) and R32_STYPE(x):
         *
         *       7 6 5 4 3 2 1 0  - bit no
         *               | | | |
         *               +-+-+-+--- source type
         *
         */

        #define NRSTYP          0x0f            /* Source type mask */
        #define NRSBYT          0x00            /* lo byte (8-bits)*/
        #define NRSSEG          0x02            /* 16-bit segment (16-bits) */
        #define NRSPTR          0x03            /* 16:16 pointer (32-bits) */
        #define NRSOFF          0x05            /* 16-bit offset (16-bits) */
        #define NRPTR48         0x06            /* 16:32 pointer (48-bits) */
        #define NROFF32         0x07            /* 32-bit offset (32-bits) */
        #define NRSOFF32        0x08            /* 32-bit self-relative offset (32-bits) */


        /*
         *  Format of NR_FLAGS(x) and R32_FLAGS(x):
         *
         *       7 6 5 4 3 2 1 0  - bit no
         *                 | | |
         *                 | +-+--- Reference type
         *                 +------- Additive fixup
         */

        #define NRADD           0x04            /* Additive fixup */
        #define NRRTYP          0x03            /* Reference type mask */
        #define NRRINT          0x00            /* Internal reference */
        #define NRRORD          0x01            /* Import by ordinal */
        #define NRRNAM          0x02            /* Import by name */
        #define NRROSF          0x03            /* Operating system fixup */


        #if (EXE386 == 0)

        /* Resource type or name string */
        struct rsrc_string
            {
            char rs_len;            /* number of bytes in string */
            char rs_string[ 1 ];    /* text of string */
            };

        #define RS_LEN( x )    (x).rs_len
        #define RS_STRING( x ) (x).rs_string

        /* Resource type information block */
        struct rsrc_typeinfo
            {
            unsigned short rt_id;
            unsigned short rt_nres;
            long rt_proc;
            };

        #define RT_ID( x )   (x).rt_id
        #define RT_NRES( x ) (x).rt_nres
        #define RT_PROC( x ) (x).rt_proc

        /* Resource name information block */
        struct rsrc_nameinfo
            {
            /* The following two fields must be shifted left by the value of  */
            /* the rs_align field to compute their actual value.  This allows */
            /* resources to be larger than 64k, but they do not need to be    */
            /* aligned on 512 byte boundaries, the way segments are           */
            unsigned short rn_offset;   /* file offset to resource data */
            unsigned short rn_length;   /* length of resource data */
            unsigned short rn_flags;    /* resource flags */
            unsigned short rn_id;       /* resource name id */
            unsigned short rn_handle;   /* If loaded, then global handle */
            unsigned short rn_usage;    /* Initially zero.  Number of times */
                                        /* the handle for this resource has */
                                        /* been given out */
            };

        #define RN_OFFSET( x ) (x).rn_offset
        #define RN_LENGTH( x ) (x).rn_length
        #define RN_FLAGS( x )  (x).rn_flags
        #define RN_ID( x )     (x).rn_id
        #define RN_HANDLE( x ) (x).rn_handle
        #define RN_USAGE( x )  (x).rn_usage

        #define RSORDID     0x8000      /* if high bit of ID set then integer id */
                                        /* otherwise ID is offset of string from
                                           the beginning of the resource table */

                                        /* Ideally these are the same as the */
                                        /* corresponding segment flags */
        #define RNMOVE      0x0010      /* Moveable resource */
        #define RNPURE      0x0020      /* Pure (read-only) resource */
        #define RNPRELOAD   0x0040      /* Preloaded resource */
        #define RNDISCARD   0xF000      /* Discard priority level for resource */

        /* Resource table */
        struct new_rsrc
            {
            unsigned short rs_align;    /* alignment shift count for resources */
            struct rsrc_typeinfo rs_typeinfo;
            };

        #define RS_ALIGN( x ) (x).rs_align


        #endif /* NOT EXE386 */

    #endif /* __NEWEXE__ */

    #ifndef DWORD
    #define DWORD long int
    #endif

    #ifndef WORD
    #define WORD  short int
    #endif

    #ifndef __EXE386__
        #define __EXE386__

        #pragma pack(1)    /* Force byte alignment */

            /*_________________________________________________________________*
             |                                                                 |
             |                                                                 |
             |  OS/2 .EXE FILE HEADER DEFINITION - 386 version 0:32            |
             |                                                                 |
             |_________________________________________________________________|
             *                                                                 */

        #define BITPERWORD      16
        #define BITPERBYTE      8
        #define OBJPAGELEN      4096
        #define E32MAGIC1       'L'        /* New magic number  "LX" */
        #define E32MAGIC2       'X'        /* New magic number  "LX" */
        #define E32MAGIC        0x584c     /* New magic number  "LX" */
        #define E32RESBYTES1    0          /* First bytes reserved */
        #define E32RESBYTES2    0          /* Second bytes reserved */
        #define E32RESBYTES3    20         /* Third bytes reserved */
        #define E32LEBO         0x00       /* Little Endian Byte Order */
        #define E32BEBO         0x01       /* Big Endian Byte Order */
        #define E32LEWO         0x00       /* Little Endian Word Order */
        #define E32BEWO         0x01       /* Big Endian Word Order */
        #define E32LEVEL        0L         /* 32-bit EXE format level */
        #define E32CPU286       0x001      /* Intel 80286 or upwardly compatibile */
        #define E32CPU386       0x002      /* Intel 80386 or upwardly compatibile */
        #define E32CPU486       0x003      /* Intel 80486 or upwardly compatibile */



        struct e32_exe                          /* New 32-bit .EXE header */
        {
            unsigned char       e32_magic[2];   /* Magic number E32_MAGIC */
            unsigned char       e32_border;     /* The byte ordering for the .EXE */
            unsigned char       e32_worder;     /* The word ordering for the .EXE */
            unsigned long       e32_level;      /* The EXE format level for now = 0 */
            unsigned short      e32_cpu;        /* The CPU type */
            unsigned short      e32_os;         /* The OS type */
            unsigned long       e32_ver;        /* Module version */
            unsigned long       e32_mflags;     /* Module flags */
            unsigned long       e32_mpages;     /* Module # pages */
            unsigned long       e32_startobj;   /* Object # for instruction pointer */
            unsigned long       e32_eip;        /* Extended instruction pointer */
            unsigned long       e32_stackobj;   /* Object # for stack pointer */
            unsigned long       e32_esp;        /* Extended stack pointer */
            unsigned long       e32_pagesize;   /* .EXE page size */
            unsigned long       e32_pageshift;  /* Page alignment shift in .EXE */
            unsigned long       e32_fixupsize;  /* Fixup section size */
            unsigned long       e32_fixupsum;   /* Fixup section checksum */
            unsigned long       e32_ldrsize;    /* Loader section size */
            unsigned long       e32_ldrsum;     /* Loader section checksum */
            unsigned long       e32_objtab;     /* Object table offset */
            unsigned long       e32_objcnt;     /* Number of objects in module */
            unsigned long       e32_objmap;     /* Object page map offset */
            unsigned long       e32_itermap;    /* Object iterated data map offset */
            unsigned long       e32_rsrctab;    /* Offset of Resource Table */
            unsigned long       e32_rsrccnt;    /* Number of resource entries */
            unsigned long       e32_restab;     /* Offset of resident name table */
            unsigned long       e32_enttab;     /* Offset of Entry Table */
            unsigned long       e32_dirtab;     /* Offset of Module Directive Table */
            unsigned long       e32_dircnt;     /* Number of module directives */
            unsigned long       e32_fpagetab;   /* Offset of Fixup Page Table */
            unsigned long       e32_frectab;    /* Offset of Fixup Record Table */
            unsigned long       e32_impmod;     /* Offset of Import Module Name Table */
            unsigned long       e32_impmodcnt;  /* Number of entries in Import Module Name Table */
            unsigned long       e32_impproc;    /* Offset of Import Procedure Name Table */
            unsigned long       e32_pagesum;    /* Offset of Per-Page Checksum Table */
            unsigned long       e32_datapage;   /* Offset of Enumerated Data Pages */
            unsigned long       e32_preload;    /* Number of preload pages */
            unsigned long       e32_nrestab;    /* Offset of Non-resident Names Table */
            unsigned long       e32_cbnrestab;  /* Size of Non-resident Name Table */
            unsigned long       e32_nressum;    /* Non-resident Name Table Checksum */
            unsigned long       e32_autodata;   /* Object # for automatic data object */
            unsigned long       e32_debuginfo;  /* Offset of the debugging information */
            unsigned long       e32_debuglen;   /* The length of the debugging info. in bytes */
            unsigned long       e32_instpreload;/* Number of instance pages in preload section of .EXE file */
            unsigned long       e32_instdemand; /* Number of instance pages in demand load section of .EXE file */
            unsigned long       e32_heapsize;   /* Size of heap - for 16-bit apps */
            unsigned long       e32_stacksize;  /* Size of stack */
            unsigned char       e32_res3[E32RESBYTES3];
                                                /* Pad structure to 196 bytes */
          };



        #define E32_MAGIC1(x)       (x).e32_magic[0]
        #define E32_MAGIC2(x)       (x).e32_magic[1]
        #define E32_BORDER(x)       (x).e32_border
        #define E32_WORDER(x)       (x).e32_worder
        #define E32_LEVEL(x)        (x).e32_level
        #define E32_CPU(x)          (x).e32_cpu
        #define E32_OS(x)           (x).e32_os
        #define E32_VER(x)          (x).e32_ver
        #define E32_MFLAGS(x)       (x).e32_mflags
        #define E32_MPAGES(x)       (x).e32_mpages
        #define E32_STARTOBJ(x)     (x).e32_startobj
        #define E32_EIP(x)          (x).e32_eip
        #define E32_STACKOBJ(x)     (x).e32_stackobj
        #define E32_ESP(x)          (x).e32_esp
        #define E32_PAGESIZE(x)     (x).e32_pagesize
        #define E32_PAGESHIFT(x)    (x).e32_pageshift
        #define E32_FIXUPSIZE(x)    (x).e32_fixupsize
        #define E32_FIXUPSUM(x)     (x).e32_fixupsum
        #define E32_LDRSIZE(x)      (x).e32_ldrsize
        #define E32_LDRSUM(x)       (x).e32_ldrsum
        #define E32_OBJTAB(x)       (x).e32_objtab
        #define E32_OBJCNT(x)       (x).e32_objcnt
        #define E32_OBJMAP(x)       (x).e32_objmap
        #define E32_ITERMAP(x)      (x).e32_itermap
        #define E32_RSRCTAB(x)      (x).e32_rsrctab
        #define E32_RSRCCNT(x)      (x).e32_rsrccnt
        #define E32_RESTAB(x)       (x).e32_restab
        #define E32_ENTTAB(x)       (x).e32_enttab
        #define E32_DIRTAB(x)       (x).e32_dirtab
        #define E32_DIRCNT(x)       (x).e32_dircnt
        #define E32_FPAGETAB(x)     (x).e32_fpagetab
        #define E32_FRECTAB(x)      (x).e32_frectab
        #define E32_IMPMOD(x)       (x).e32_impmod
        #define E32_IMPMODCNT(x)    (x).e32_impmodcnt
        #define E32_IMPPROC(x)      (x).e32_impproc
        #define E32_PAGESUM(x)      (x).e32_pagesum
        #define E32_DATAPAGE(x)     (x).e32_datapage
        #define E32_PRELOAD(x)      (x).e32_preload
        #define E32_NRESTAB(x)      (x).e32_nrestab
        #define E32_CBNRESTAB(x)    (x).e32_cbnrestab
        #define E32_NRESSUM(x)      (x).e32_nressum
        #define E32_AUTODATA(x)     (x).e32_autodata
        #define E32_DEBUGINFO(x)    (x).e32_debuginfo
        #define E32_DEBUGLEN(x)     (x).e32_debuglen
        #define E32_INSTPRELOAD(x)  (x).e32_instpreload
        #define E32_INSTDEMAND(x)   (x).e32_instdemand
        #define E32_HEAPSIZE(x)     (x).e32_heapsize
        #define E32_STACKSIZE(x)    (x).e32_stacksize



        /*
         *  Format of E32_MFLAGS(x):
         *
         *  Low word has the following format:
         *
         *  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
         *   |     |          | |     | | | |
         *   |     |          | |     | | | +------- Per-Process Library Initialization
         *   |     |          | |     | | +--------- SystemDLL (internal fixups discarded)
         *   |     |          | |     | +----------- No Internal Fixups for Module in .EXE
         *   |     |          | |     +------------- No External Fixups for Module in .EXE
         *   |     |          | +------------------- Incompatible with PM Windowing
         *   |     |          +--------------------- Compatible with PM Windowing
         *   |     |                                 Uses PM Windowing API
         *   |     +-------------------------------- Module not Loadable
         *   +-------------------------------------- Library Module
         */


        #define E32NOTP          0x8000L        /* Library Module - used as NENOTP */
        #define E32NOLOAD        0x2000L        /* Module not Loadable */
        #define E32PMAPI         0x0300L        /* Uses PM Windowing API */
        #define E32PMW           0x0200L        /* Compatible with PM Windowing */
        #define E32NOPMW         0x0100L        /* Incompatible with PM Windowing */
        #define E32NOEXTFIX      0x0020L        /* NO External Fixups in .EXE */
        #define E32NOINTFIX      0x0010L        /* NO Internal Fixups in .EXE */
        #define E32SYSDLL        0x0008L        /* System DLL, Internal Fixups discarded*/
        #define E32LIBINIT       0x0004L        /* Per-Process Library Initialization */
        #define E32LIBTERM       0x40000000L    /* Per-Process Library Termination */
        #define E32APPMASK       0x0300L        /* Application Type Mask */


        /*
         *  Format of E32_MFLAGS(x):
         *
         *  High word has the following format:
         *
         *  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
         *                                    | |
         *                                    | +--- Protected memory library module
         *                                    +----- Device driver
         */

        #define E32PROTDLL       0x10000L       /* Protected memory library module */
        #define E32DEVICE        0x20000L       /* Device driver                   */
        #define E32MODEXE        0x00000L       /* .EXE module                     */
        #define E32MODDLL        0x08000L       /* .DLL module                     */
        #define E32MODPROTDLL    0x18000L       /* Protected memory library module */
        #define E32MODPDEV       0x20000L       /* Physical device driver          */
        #define E32MODVDEV       0x28000L       /* Virtual device driver           */
        #define E32MODMASK       0x38000L       /* Module type mask                */

        /*
         *  RELOCATION DEFINITIONS - RUN-TIME FIXUPS
         */

        typedef union _RELOC_OFS
        {
            unsigned short offset16;
            unsigned long  offset32;
        } RELOC_OFS;                            /* 16-bit or 32-bit offset */


        /***ET+ r32_rlc - Relocation item */

        struct r32_rlc                          /* Relocation item */
        {
            unsigned char       nr_stype;       /* Source type - field shared with new_rlc */
            unsigned char       nr_flags;       /* Flag byte - field shared with new_rlc */
            short               r32_soff;       /* Source offset */
            unsigned short      r32_objmod;     /* Target object number or Module ordinal */

            union targetid
            {
                RELOC_OFS         intref;      /* Internal fixup */

                union extfixup
                {
                    RELOC_OFS      proc;        /* Procedure name offset */
                    unsigned long  ord;         /* Procedure odrinal */
                } extref;      /* External fixup */

                struct addfixup
                {
                    unsigned short entry;       /* Entry ordinal */
                    RELOC_OFS      addval;      /* Value added to the address */
                } addfix;      /* Additive fixup */
            }
                                r32_target;     /* Target data */
            unsigned short      r32_srccount;   /* Number of chained fixup records */
            unsigned short      r32_chain;      /* Chain head */
        };

        /*
         *  In 32-bit .EXE file run-time relocations are written as varying size
         *  records, so we need many size definitions.
         */

        #define RINTSIZE16      8
        #define RINTSIZE32      10
        #define RORDSIZE        8
        #define RNAMSIZE16      8
        #define RNAMSIZE32      10
        #define RADDSIZE16      10
        #define RADDSIZE32      12

        #if FALSE
        /*
         *  Access macros defined in NEWEXE.H !!!
         */
        #define NR_STYPE(x)      (x).nr_stype
        #define NR_FLAGS(x)      (x).nr_flags
        #endif

        #define R32_SOFF(x)      (x).r32_soff
        #define R32_OBJNO(x)     (x).r32_objmod
        #define R32_MODORD(x)    (x).r32_objmod
        #define R32_OFFSET16(x)  (x).r32_target.intref.offset16
        #define R32_OFFSET32(x)  (x).r32_target.intref.offset32
        #define R32_PROCOFF16(x) (x).r32_target.extref.proc.offset16
        #define R32_PROCOFF32(x) (x).r32_target.extref.proc.offset32
        #define R32_PROCORD(x)   (x).r32_target.extref.ord
        #define R32_ENTRY(x)     (x).r32_target.addfix.entry
        #define R32_ADDVAL16(x)  (x).r32_target.addfix.addval.offset16
        #define R32_ADDVAL32(x)  (x).r32_target.addfix.addval.offset32
        #define R32_SRCCNT(x)    (x).r32_srccount
        #define R32_CHAIN(x)     (x).r32_chain



        /*
         *  Format of NR_STYPE(x)
         *
         *       7 6 5 4 3 2 1 0  - bit no
         *           | | | | | |
         *           | | +-+-+-+--- Source type
         *           | +----------- Fixup to 16:16 alias
         *           +------------- List of source offset follows fixup record
         */

        #if FALSE

                    /* DEFINED in newexe.h !!! */

        #define NRSTYP          0x0f            /* Source type mask */
        #define NRSBYT          0x00            /* lo byte (8-bits)*/
        #define NRSSEG          0x02            /* 16-bit segment (16-bits) */
        #define NRSPTR          0x03            /* 16:16 pointer (32-bits) */
        #define NRSOFF          0x05            /* 16-bit offset (16-bits) */
        #define NRPTR48         0x06            /* 16:32 pointer (48-bits) */
        #define NROFF32         0x07            /* 32-bit offset (32-bits) */
        #define NRSOFF32        0x08            /* 32-bit self-relative offset (32-bits) */
        #endif


        #define NRSRCMASK       0x0f            /* Source type mask */
        #define NRALIAS         0x10            /* Fixup to alias */
        #define NRCHAIN         0x20            /* List of source offset follows */
                                                /* fixup record, source offset field */
                                                /* in fixup record contains number */
                                                /* of elements in list */

        /*
         *  Format of NR_FLAGS(x) and R32_FLAGS(x):
         *
         *       7 6 5 4 3 2 1 0  - bit no
         *       | | | |   | | |
         *       | | | |   | +-+--- Reference type
         *       | | | |   +------- Additive fixup
         *       | | | +----------- 32-bit Target Offset Flag (1 - 32-bit; 0 - 16-bit)
         *       | | +------------- 32-bit Additive Flag (1 - 32-bit; 0 - 16-bit)
         *       | +--------------- 16-bit Object/Module ordinal (1 - 16-bit; 0 - 8-bit)
         *       +----------------- 8-bit import ordinal (1 - 8-bit;
         *                                                0 - NR32BITOFF toggles
         *                                                    between 16 and 32 bit
         *                                                    ordinal)
         */

        #ifdef NEVER_INCLUDE_THE_FOLLWING
                    /* DEFINED in newexe.h !!! */
            #define NRRTYP          0x03            /* Reference type mask */
            #define NRRINT          0x00            /* Internal reference */
            #define NRRORD          0x01            /* Import by ordinal */
            #define NRRNAM          0x02            /* Import by name */
            #define NRADD           0x04            /* Additive fixup */
        #endif

        #define NRRENT          0x03            /* Internal entry table fixup */

        #define NR32BITOFF      0x10            /* 32-bit Target Offset */
        #define NR32BITADD      0x20            /* 32-bit Additive fixup */
        #define NR16OBJMOD      0x40            /* 16-bit Object/Module ordinal */
        #define NR8BITORD       0x80            /* 8-bit import ordinal */
        /*end*/

        /*
         *  Data structures for storing run-time fixups in linker virtual memory.
         *
         *  Each object has a list of Object Page Directories which specify
         *  fixups for given page. Each page has its own hash table which is
         *  used to detect fixups to the same target.
         */

        #define PAGEPERDIR      62
        #define LG2DIR          7


        typedef struct _OBJPAGEDIR
        {
            DWORD   next;                       /* Virtual pointer to next dir on list */
            WORD    ht[PAGEPERDIR];             /* Pointers to individual hash tables */
        }
            OBJPAGEDIR;

        /*
         *  OBJECT TABLE
         */

        /***ET+ o32_obj Object Table Entry */

        struct o32_obj                          /* Flat .EXE object table entry */
        {
            unsigned long       o32_size;       /* Object virtual size */
            unsigned long       o32_base;       /* Object base virtual address */
            unsigned long       o32_flags;      /* Attribute flags */
            unsigned long       o32_pagemap;    /* Object page map index */
            unsigned long       o32_mapsize;    /* Number of entries in object page map */
            unsigned long       o32_reserved;   /* Reserved */
        };


        #define O32_SIZE(x)     (x).o32_size
        #define O32_BASE(x)     (x).o32_base
        #define O32_FLAGS(x)    (x).o32_flags
        #define O32_PAGEMAP(x)  (x).o32_pagemap
        #define O32_MAPSIZE(x)  (x).o32_mapsize
        #define O32_RESERVED(x) (x).o32_reserved

        /*
         *  Format of O32_FLAGS(x)
         *
         *  High word of dword flag field is not used for now.
         *  Low word has the following format:
         *
         *  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
         *   |  |  |  |     | | | | | | | | | | |
         *   |  |  |  |     | | | | | | | | | | +--- Readable Object
         *   |  |  |  |     | | | | | | | | | +----- Writeable Object
         *   |  |  |  |     | | | | | | | | +------- Executable Object
         *   |  |  |  |     | | | | | | | +--------- Resource Object
         *   |  |  |  |     | | | | | | +----------- Object is Discardable
         *   |  |  |  |     | | | | | +------------- Object is Shared
         *   |  |  |  |     | | | | +--------------- Object has preload pages
         *   |  |  |  |     | | | +----------------- Object has invalid pages
         *   |  |  |  |     | | +------------------- Object is permanent and swappable
         *   |  |  |  |     | +--------------------- Object is permanent and resident
         *   |  |  |  |     +----------------------- Object is permanent and long lockable
         *   |  |  |  +----------------------------- 16:16 alias required (80x86 specific)
         *   |  |  +-------------------------------- Big/Default bit setting (80x86 specific)
         *   |  +----------------------------------- Object is conforming for code (80x86 specific)
         *   +-------------------------------------- Object I/O privilege level (80x86 specific)
         *
         */

        #define OBJREAD         0x0001L             /* Readable Object   */
        #define OBJWRITE        0x0002L             /* Writeable Object  */
        #define OBJRSRC         0x0008L             /* Resource Object   */
        #define OBJINVALID      0x0080L             /* Object has invalid pages  */
        #define LNKNONPERM      0x0600L             /* Object is nonpermanent - should be */
        #define OBJNONPERM      0x0000L             /* zero in the .EXE but LINK386 uses 6 */
        #define OBJPERM         0x0100L             /* Object is permanent and swappable */
        #define OBJRESIDENT     0x0200L             /* Object is permanent and resident */
        #define OBJCONTIG       0x0300L             /* Object is resident and contiguous */
        #define OBJDYNAMIC      0x0400L             /* Object is permanent and long locable */
        #define OBJTYPEMASK     0x0700L             /* Object type mask */
        #define OBJALIAS16      0x1000L             /* 16:16 alias required (80x86 specific)           */
        #define OBJBIGDEF       0x2000L             /* Big/Default bit setting (80x86 specific)        */
        #define OBJIOPL         0x8000L             /* Object I/O privilege level (80x86 specific)     */
        #if FOR_EXEHDR
        /*
         *  Name these flags differently for EXEHDR.EXE - avoid conflicts with 286 version
         */
        #define OBJDISCARD       0x0010L            /* Object is Discardable */
        #define OBJSHARED        0x0020L            /* Object is Shared */
        #define OBJPRELOAD       0x0040L            /* Object has preload pages  */
        #define OBJEXEC          0x0004L            /* Executable Object */
        #define OBJCONFORM       0x4000L            /* Object is conforming for code (80x86 specific)  */
        #else
        /*
         *  Life will be easier, if we keep the same names for the following flags:
         */
        #ifndef NSDISCARD
            #define NSDISCARD       0x0010L             /* Object is Discardable */
        #endif
        #ifndef NSMOVE
            #define NSMOVE          NSDISCARD           /* Moveable object is for sure Discardable */
        #endif
        #ifndef NSSHARED
            #define NSSHARED        0x0020L             /* Object is Shared */
        #endif
        #ifndef NSPRELOAD
            #define NSPRELOAD       0x0040L             /* Object has preload pages  */
        #endif
        #ifndef NSEXRD
            #define NSEXRD          0x0004L             /* Executable Object */
        #endif
        #ifndef NSCONFORM
            #define NSCONFORM       0x4000L             /* Object is conforming for code (80x86 specific)  */
        #endif
        #endif
        /*end*/

        /***ET+ o32_map - Object Page Map entry */

        struct o32_map                              /* Object Page Table entry */
        {
            unsigned long   o32_pagedataoffset;     /* file offset of page */
            unsigned short  o32_pagesize;           /* # bytes of page data */
            unsigned short  o32_pageflags;          /* Per-Page attributes */
        };


        #define GETPAGEIDX(x)    ((x).o32_pagedataoffset)

        #define PUTPAGEIDX(x,i)  ((x).o32_pagedataoffset = ((unsigned long)(i)))

        #define PUTPAGESIZ(x,i)  ((x).o32_pagesize = ((unsigned int)(i)))

        #define GETPAGESIZ(x)    ((x).o32_pagesize)

        #define PAGEFLAGS(x)    (x).o32_pageflags


        #define VALID           0x0000                /* Valid Physical Page in .EXE */
        #define ITERDATA        0x0001                /* Iterated Data Page */
        #define INVALID         0x0002                /* Invalid Page */
        #define ZEROED          0x0003                /* Zero Filled Page */
        #define RANGE           0x0004                /* Range of pages */
        #define ITERDATA2       0x0005                /* Iterated Data Page Type II */
        /*end*/

        /*
         *  RESOURCE TABLE
         */

        /***ET+ rsrc32 - Resource Table Entry */

        struct rsrc32                               /* Resource Table Entry */
        {
            unsigned short      type;               /* Resource type */
            unsigned short      name;               /* Resource name */
            unsigned long       cb;                 /* Resource size */
            unsigned short      obj;                /* Object number */
            unsigned long       offset;             /* Offset within object */
        };
        /*end*/


         /*
          * Iteration Record format for 'EXEPACK'ed pages.
          */
        struct LX_Iter
        {
            unsigned short LX_nIter;            /* number of iterations */
            unsigned short LX_nBytes;           /* number of bytes */
            unsigned char  LX_Iterdata;         /* iterated data byte(s) */
        };


        /*
         *  ENTRY TABLE DEFINITIONS
         */

        /***ET+ b32_bundle - Entry Table */

        struct b32_bundle
        {
            unsigned char       b32_cnt;        /* Number of entries in this bundle */
            unsigned char       b32_type;       /* Bundle type */
            unsigned short      b32_obj;        /* Object number */
        };                                      /* Follows entry types */

        struct e32_entry
        {
            unsigned char       e32_flags;      /* Entry point flags */
            union entrykind
            {
                RELOC_OFS       e32_offset;     /* 16-bit/32-bit offset entry */
                struct callgate
                {
                    unsigned short offset;      /* Offset in segment */
                    unsigned short callgate;    /* Callgate selector */
                }
                                e32_callgate;   /* 286 (16-bit) call gate */
                struct fwd
                {
                    unsigned short  modord;     /* Module ordinal number */
                    unsigned long   value;      /* Proc name offset or ordinal */
                }
                                e32_fwd;        /* Forwarder */
            }
                                e32_variant;    /* Entry variant */
        };



        #define B32_CNT(x)      (x).b32_cnt
        #define B32_TYPE(x)     (x).b32_type
        #define B32_OBJ(x)      (x).b32_obj

        #define E32_EFLAGS(x)   (x).e32_flags
        #define E32_OFFSET16(x) (x).e32_variant.e32_offset.offset16
        #define E32_OFFSET32(x) (x).e32_variant.e32_offset.offset32
        #define E32_GATEOFF(x)  (x).e32_variant.e32_callgate.offset
        #define E32_GATE(x)     (x).e32_variant.e32_callgate.callgate
        #define E32_MODORD(x)   (x).e32_variant.e32_fwd.modord
        #define E32_VALUE(x)    (x).e32_variant.e32_fwd.value

        #define FIXENT16        3
        #define FIXENT32        5
        #define GATEENT16       5
        #define FWDENT          7

        /*
         *  BUNDLE TYPES
         */

        #define EMPTY        0x00               /* Empty bundle */
        #define ENTRY16      0x01               /* 16-bit offset entry point */
        #define GATE16       0x02               /* 286 call gate (16-bit IOPL) */
        #define ENTRY32      0x03               /* 32-bit offset entry point */
        #define ENTRYFWD     0x04               /* Forwarder entry point */
        #define TYPEINFO     0x80               /* Typing information present flag */


        /*
         *  Format for E32_EFLAGS(x)
         *
         *       7 6 5 4 3 2 1 0  - bit no
         *       | | | | | | | |
         *       | | | | | | | +--- exported entry
         *       | | | | | | +----- uses shared data
         *       +-+-+-+-+-+------- parameter word count
         */

        #define E32EXPORT       0x01            /* Exported entry */
        #define E32SHARED       0x02            /* Uses shared data */
        #define E32PARAMS       0xf8            /* Parameter word count mask */

        /*
         *  Flags for forwarders only:
         */

        #define FWD_ORDINAL     0x01            /* Imported by ordinal */


        #pragma pack()       /* Restore default alignment */

        /*end*/

    #endif /* __EXE386__ */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
