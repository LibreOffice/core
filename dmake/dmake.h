/* RCS  $Id: dmake.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Global defines for dmake.
--
-- DESCRIPTION
--  All the interesting bits and flags that dmake uses are defined here.
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
-- WWW
--      http://dmake.wticorp.com/
--
-- COPYRIGHT
--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.
--
--      This program is NOT free software; you can redistribute it and/or
--      modify it under the terms of the Software License Agreement Provided
--      in the file <distribution-root>/readme/license.txt.
--
-- LOG
--      Use cvs log to obtain detailed change logs.
*/

#ifndef _DMAKE_INCLUDED_
#define _DMAKE_INCLUDED_

#define MAX_INC_DEPTH     10     /* max of ten nested include files      */
#define MAX_COND_DEPTH    20     /* max nesting level of conditionals    */
#define ERROR_EXIT_VALUE  255    /* return code of aborted make          */
#define CONTINUATION_CHAR '\\'   /* line continuation \<nl>              */
#define DEF_ESCAPE_CHAR   '\\'   /* escape char for used chars           */
#define ESCAPE_CHAR       *Escape_char
#define COMMENT_CHAR      '#'    /* start of comment chars               */
#define TGT_DEP_SEP       ':'    /* separator for targets and dependents */
#define CONDSTART         '.'    /* start of conditional token eg .IF    */
#define DEF_MAKE_PNAME    "dmake"/* default name to use as name of make  */


/* ............... Hashing function constants ......................... */
#define HASH_TABLE_SIZE  200            /* See hash.c for description    */


/* Bit flags for cells and macro definitions. */
#define M_DEFAULT        0x0000         /* default flag value           */
#define M_MARK           0x0001         /* mark for circularity checks  */
#define M_PRECIOUS       0x0002         /* keep macro, same as A_PRE... */
#define M_MULTI          0x0004         /* multiple redefinitions ok!   */
#define M_EXPANDED       0x0008         /* macro has been assigned      */
#define M_USED           0x0010         /* macro has been expanded      */
#define M_LITERAL        0x0020         /* don't strip w/s on macro def */
#define M_NOEXPORT       0x0040         /* don't export macro for -x    */
#define M_FORCE          0x0080         /* Force a macro redefinition   */
#define M_PUSH           0x0100         /* Push previous macro defintn  */
#define M_INIT           0x0200         /* Macro defined initially      */
#define M_VAR_BIT        0x1000         /* macro bit variable           */
#define M_VAR_CHAR       0x2000         /* macro char variable          */
#define M_VAR_STRING     0x4000         /* macro string variable        */
#define M_VAR_INT        0x8000         /* macro integer variable       */

#define M_VAR_MASK       0xf000         /* macro variable mask          */



/* Global and target attribute flag definitions.
 * If you change the values of these or re-order them make appropriate changes
 * in dump.c so that the output of dmake -p matches the attribute info for a
 * target. */

#define A_DEFAULT        0x00000        /* default flag value           */
#define A_PRECIOUS       0x00001        /* object is precious           */
#define A_SILENT         0x00002        /* don't echo commands          */
#define A_LIBRARY        0x00004        /* target is an archive         */
#define A_EPILOG         0x00008        /* insert shell epilog code     */
#define A_PROLOG         0x00010        /* insert shell prolog code     */
#define A_IGNORE         0x00020        /* ignore errors                */
#define A_SYMBOL         0x00040        /* lib member is a symbol       */
#define A_NOINFER        0x00080        /* no trans closure from cell   */
#define A_UPDATEALL      0x00100        /* all targets of rule modified */
#define A_SEQ            0x00200        /* sequential make attribute    */
#define A_SETDIR         0x00400        /* cd to dir when making target */
#define A_SHELL          0x00800        /* run the recipe using a shell */
#define A_SWAP           0x01000        /* swap on exec.                */
#define A_MKSARGS        0x02000        /* use MKS argument swapping    */
#define A_PHONY          0x04000        /* .PHONY attribute             */
#define A_NOSTATE        0x08000        /* don't track state for me     */
#define A_IGNOREGROUP    0x10000        /* Ignore group recipe          */
#define A_EXECUTE        0x20000        /* execute this recipe under -n */
#define A_ERRREMOVE      0x40000        /* remove this target if error  */
#define MAX_ATTR         A_ERRREMOVE    /* highest valid attribute      */
#define A_LIBRARYM       0x80000        /* target is an archive member  */
#define A_FRINGE        0x100000        /* cell is on the fringe        */
#define A_COMPOSITE     0x200000        /* member of lib(targ) name     */
#define A_FFNAME        0x400000        /* if set, free ce_fname in stat*/
#define A_UPDATED       0x800000        /* Used to mark cell as updated */
#define A_ROOT        0x01000000        /* True if it is a root prereq  */
#define A_GROUP       0x02000000        /* True if rule is to be a group*/
#define A_WHATIF      0x04000000        /* used for WHATIF tests        */
#define A_POOL        0x08000000        /* used for directory pool      */
#define A_ERROR       0x10000000        /* used to halt construction    */
#define A_FIRST       0x20000000        /* used for .INCLUDE termination*/


/* Global and target bit flag definitions */

#define F_DEFAULT        0x0000         /* default flag value           */
#define F_MARK           0x0001         /* circularity check mark       */
#define F_MULTI          0x0002         /* multiple rules for target    */
#define F_SINGLE         0x0004         /* exec rules one/prerequisite  */
#define F_TARGET         0x0008         /* marks a target               */
#define F_RULES          0x0010         /* indicates target has rules   */
#define F_GROUP          0x0020         /* indicates that rules are to  */
                                       /* fed to the shell as a group  */

#define F_TRANS          0x0040         /* same as F_STAT not used tgthr*/
#define F_STAT           0x0040         /* target already stated        */
#define F_VISITED        0x0080         /* target scheduled for make    */
#define F_USED           0x0080         /* used in rulparse.c           */
#define F_SPECIAL        0x0100         /* marks a special target       */
#define F_DFA            0x0200         /* bit for marking added DFA    */
#define F_EXPLICIT       0x0400         /* explicit target in makefile  */
#define F_PERCENT        0x0800         /* marks a target as a % rule   */
#define F_REMOVE         0x1000         /* marks an intermediate target */
#define F_MAGIC          0x2000         /* marks a magic target         */
#define F_INFER          0x4000         /* target is result of inference*/
#define F_MADE           0x8000         /* target is manufactured       */


/* Definitions for the Parser states */
#define NORMAL_SCAN    0      /* normal processing state */
#define RULE_SCAN      1      /* scan of rule text       */

/* definitions for macro operator types */
#define M_OP_EQ   1           /* macro operation is '='   0000 0001 */
#define M_OP_CL   3           /* macro operation is ':='  0000 0011 */
#define M_OP_PL   5           /* macro operation is '+='  0000 0101 */
#define M_OP_DF   9           /* macro operation is '*='  0000 1001 */
#define M_OP_PLCL 7           /* macro operation is '+:=' 0000 0111 */
#define M_OP_DFCL 11          /* macro operation is '*:=' 0000 1011 */
#define M_OP_CM   17          /* macro operation is '?='  0001 0001 */
#define M_OP_SI   32          /* macro operation is '!'   0010 ---- */

/* definitions for rule operator types */
#define R_OP_CL   1           /* rule operation is ':'   */
#define R_OP_DCL  2           /* rule operation is '::'  */
#define R_OP_BG   4           /* rule operation is ':!'  */
#define R_OP_UP   8           /* rule operation is ':^'  */
#define R_OP_MI  16           /* rule operation is ':-'  */
#define R_OP_OR  32           /* rule operation is ':|'  */

/* definitions for modifier application in Apply_modifiers in expand.c */
#define SUFFIX_FLAG       1   /* defines for macro modifier code */
#define DIRECTORY_FLAG    2
#define FILE_FLAG         4
#define WHOLENAME_FLAGS   7
#define TOLOWER_FLAG      8
#define TOUPPER_FLAG     16
#define INFNAME_FLAG     32
#define JUST_FIRST_FLAG  64

/* special target definitions for use inside dmake */
#define ST_IF           1
#define ST_ELSE         2
#define ST_END          3
#define ST_REST         4       /* remaining special targets */
#define ST_INCLUDE      5
#define ST_SOURCE       7
#define ST_EXPORT       8
#define ST_IMPORT       9
#define ST_ELIF        10
#define ST_KEEP        11
#define ST_EXIT        12
#define ST_IFEQ        13
#define ST_IFNEQ       14

/* Flags for controling use of -v switch */
#define V_NONE          0x00
#define V_LEAVE_TMP     0x01
#define V_DIR_SET       0x02
#define V_DIR_CACHE     0x04
#define V_INFER         0x08
#define V_MAKE          0x10
#define V_FILE_IO       0x20
#define V_WARNALL       0x40
#define V_ALL           (V_LEAVE_TMP | V_DIR_SET | V_INFER | V_MAKE |\
                       V_FILE_IO | V_DIR_CACHE | V_WARNALL)

/* Macro definitions for use inside dmake */
#define SET_TOKEN(A, B)  (A)->tk_str = (B);\
                        (A)->tk_cchar = *(B);\
                        (A)->tk_quote = 1;

#define CLEAR_TOKEN(A)   *(A)->tk_str = (A)->tk_cchar
#define GET_MACRO(A)     Get_name(A, Macs, FALSE)
#define iswhite(C)       ((C == ' ') || (C == '\t'))
#define STOBOOL(A)       (A && ((*A | 0x20) == 'y'))

#endif

