/* RCS  $Id: struct.h,v 1.1.1.1 2000-09-22 15:33:25 hr Exp $
--
-- SYNOPSIS
--      Structure definitions
--
-- DESCRIPTION
--  dmake main data structure definitions.  See each of the individual
--  struct declarations for more detailed information on the defined
--  fields and their use.
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

#ifndef _STRUCT_INCLUDED_
#define _STRUCT_INCLUDED_

typedef uint32 t_attr;

/* The following struct is the cell used in the hash table.
 * NOTE:  It contains the actual hash value.  This allows the hash table
 *        insertion to compare hash values and to do a string compare only
 *        for entries that have matching hash_key values.  This elliminates
 *        99.9999% of all extraneous string compare operations when searching
 *        a hash table chain for matching entries.  */

typedef struct hcell {
    struct hcell    *ht_next;   /* next entry in the hash table */
        struct hcell    *ht_link;       /* for temporary lists */
    char        *ht_name;   /* name of this cell */
    char        *ht_value;  /* cell value if any */
    uint32      ht_hash;    /* actual hash_key of cell */
    int     ht_flag;    /* flags belonging to hash entry */

    /* NOTE: some macros have corresponding variables defined
     * that control program behaviour.  For these macros a
     * bit of ht_flag indicates the variable value will be set, and the
     * type of the value that will be set.
     *
     * The struct below contains a mask for bit variables, and a
     * pointer to the global STATIC location for that variable.
     * String and char variables point to the same place as ht_value
     * and must be updated when ht_value changes, bit variables must
     * have their value recomputed. See Def_macro code for more
     * details.
     *
     * NOTE:  Macro variables and Targets are always distinct.  Thus
     * the value union contains pointers back at cells that own
     * a particular name entry.  A conflict in this can never
     * arise, ie pointers at cells will never be used as
     * values for a macro variable, since the cell and macro
     * name spaces are completely distinct. */

    struct {
        int mv_mask;    /* bit mask for bit variable      */
        union {
             char** mv_svar;/* ptr to string valued glob var  */
            char*   mv_cvar;/* ptr to char   valued glob var */
            t_attr* mv_bvar;/* ptr to bit    valued glob var */
             int*    mv_ivar;/* ptr to int    valued glob var  */

            struct {
               struct tcell* ht_owner;/* ptr to CELL owning name */
               struct tcell* ht_root; /* root ptr for explode */
            } ht;
        } val;
    } var;              /* variable's static equivalent */
} HASH, *HASHPTR;

#define MV_MASK   var.mv_mask
#define MV_SVAR   var.val.mv_svar
#define MV_CVAR   var.val.mv_cvar
#define MV_BVAR   var.val.mv_bvar
#define MV_IVAR   var.val.mv_ivar
#define CP_OWNR   var.val.ht.ht_owner
#define CP_ROOT   var.val.ht.ht_root



/* This struct holds the list of temporary files that have been created.
 * It gets unlinked when Quit is called due to an execution error */
typedef struct flst {
   char     *fl_name;   /* file name        */
   FILE     *fl_file;   /* the open file    */
   struct flst  *fl_next;   /* pointer to next file */
} FILELIST, *FILELISTPTR;


/* The next struct is used to link together prerequisite lists */
typedef struct lcell {
    struct tcell    *cl_prq;    /* link to a prerequisite   */
    struct lcell    *cl_next;   /* next cell on dependency list */
    int     cl_flag;    /* flags for link cell      */
} LINK, *LINKPTR;


/* This is the structure of a target cell in the dag which represents the
 * graph of dependencies.  Each possible target is represented as a cell.
 *
 * Each cell contains a pointer to the hash table entry for this cell.
 * The hash table entry records the name of the cell. */

typedef struct tcell {
    struct hcell    *ce_name;   /* name of this cell                */
        struct hcell    *ce_pushed;     /* local pushed macro definitions   */

        struct lcell    ce_all;         /* link for grouping UPDATEALL cells*/
        struct tcell    *ce_set;        /* set rep. valid if ce_all != NULL */
    struct tcell    *ce_setdir; /* SETDIR ROOT pointer for this cell*/
    struct tcell    *ce_link;   /* link for temporary list making   */
        struct tcell    *ce_parent;     /* used by inner loop, not a static */

    struct lcell    *ce_prq;    /* list of prerequisites for cell   */
    struct lcell    *ce_prqorg; /* list of original prerequisites   */
    struct lcell    *ce_indprq; /* indirect prerequisites for % cell*/

    struct str      *ce_recipe; /* recipe for making this cell      */
    FILELISTPTR     ce_files;   /* list of temporary files for cell */
        struct str      *ce_cond;       /* conditional macro assignments    */

    char        *ce_per;    /* value of % in %-meta expansion   */
    char        *ce_fname;  /* file name associated with target */
    char        *ce_lib;    /* archive name, if A_LIBRARYM      */
    char        *ce_dir;    /* value for .SETDIR attribute      */

    int     ce_count;   /* value for :: recipe set          */
    int     ce_index;   /* value of count for next :: child */
    int         ce_flag;    /* all kinds of goodies         */
    t_attr      ce_attr;    /* attributes for this target       */
    time_t      ce_time;    /* time stamp value of target if any*/
} CELL, *CELLPTR;

#define CE_NAME     ce_name->ht_name
#define CE_RECIPE       ce_recipe
#define CE_PRQ          ce_prq
#define CeMeToo(C)      &((C)->ce_all)
#define CeNotMe(C)      (C)->ce_all.cl_next


/* This struct represents that used by Get_token to return and control
 * access to a token list inside a particular string.  This gives the
 * ability to access non overlapping tokens simultaneously from
 * multiple strings. */

typedef struct {
    char *tk_str;              /* the string to search for tokens  */
    char tk_cchar;             /* current char under *str          */
    int  tk_quote;             /* if we are scanning a quoted str  */
}  TKSTR, *TKSTRPTR;



/* Below is the struct used to represent a string.  It points at possibly
 * another string, since the set of rules for making a target is a collection
 * of strings. */


typedef struct str {
    char        *st_string; /* the string value */
    struct str  *st_next;   /* pointer to the next string */
    t_attr      st_attr;    /* attr for rule operations */
} STRING, *STRINGPTR;



/* These structs are used in processing of the % rules, and in building
 * the NFA machine that is used to match an arbitrary target string to
 * one of the % rules that is represented by each DFA */

typedef int16 statecnt;     /* limits the max number of dfa states  */


/* Each state of the DFA contains four pieces of information. */
typedef struct st {
    struct st   *no_match;  /* state to go to if no match */
    struct st   *match;     /* state to go to if we do match */
    char        symbol;     /* symbol on which we transit */
    char        action;     /* action to perform if match */
} STATE, *STATEPTR;


/* Each DFA machine looks like this.  It must have two pointers that represent
 * the value of % in the matched string, and it contains a pointer into the
 * current state, as well as the array of all states. */
typedef struct {
    char        *pstart;    /* start of % string match */
    char        *pend;      /* end of % string match */
    STATEPTR    c_state;    /* current DFA state */
    CELLPTR     node;       /* % target represented by this DFA */
    STATEPTR    states;     /* table of states for the DFA */
} DFA, *DFAPTR;


/* An NFA is a collection of DFA's.  For each DFA we must know it's current
 * state and where the next NFA is. */
typedef struct nfa_machine {
    DFAPTR      dfa;        /* The DFA for this eps transition */
    char        status;     /* DFA state */
    struct nfa_machine *next;   /* the next DFA in NFA */
} NFA, *NFAPTR;



/* The next struct is used to link together DFA nodes for inference. */

typedef struct dfal {
    struct tcell    *dl_meta;   /* link to %-meta cell      */
    struct dfal *dl_next;   /* next cell on matched DFA list*/
    struct dfal *dl_prev;   /* prev cell on matched DFA list*/
    struct dfal *dl_member; /* used during subset calc  */
    char        dl_delete;  /* used during subset calc  */
    char        *dl_per;    /* value of % for matched DFA   */
    statecnt        dl_state;   /* matched state of the DFA */
    int     dl_prep;    /* repetion count for the cell  */
} DFALINK, *DFALINKPTR;


/* This struct is used to store the stack of DFA sets during inference */
typedef struct dfst {
   DFALINKPTR   df_set;         /* pointer to the set       */
   struct dfst *df_next;        /* next element in the stack    */
} DFASET, *DFASETPTR;


/* We need sets of items during inference, here is the item, we form sets
 * by linking them together. */

typedef struct ic {
   CELLPTR  ic_meta;        /* Edge we used to make this cell*/
   DFALINKPTR   ic_dfa;         /* Dfa that we matched against   */
   CELLPTR  ic_setdirroot;      /* setdir root pointer for cell  */
   DFASET       ic_dfastack;        /* set of dfas we're working with*/
   int      ic_dmax;        /* max depth of cycles in graph  */
   char        *ic_name;        /* name of the cell to insert    */
   char        *ic_dir;         /* dir to CD to prior to recurse */
   struct ic   *ic_next;        /* next pointer to link      */
   struct ic   *ic_link;        /* link all ICELL'S together     */
   struct ic   *ic_parent;      /* pointer to post-requisite     */
   char     ic_flag;        /* flag, used for NOINFER only   */
   char         ic_exists;      /* TRUE if prerequisite exists   */
} ICELL, *ICELLPTR;

#endif
