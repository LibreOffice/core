/*************************************************************************
 *
 *  $RCSfile: public.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:02:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _DMAKE_PUBLIC_h
#define _DMAKE_PUBLIC_h

#ifdef EXTERN
#undef EXTERN
#endif
#if defined(DEFINE_DMAKE_VARIABLES)
#define EXTERN
#else
#define EXTERN extern
#endif

/***** genpub: Begin list of generated function headers */
void Infer_recipe ANSI((CELLPTR, CELLPTR));
int Make_targets ANSI(());
int Make ANSI((CELLPTR, CELLPTR));
int Exec_commands ANSI((CELLPTR));
void Print_cmnd ANSI((char *, int, int));
int Push_dir ANSI((char *, char *, int));
void Pop_dir ANSI((int));
void Append_line ANSI((char *, int, FILE *, char *, int, int));
void Stat_target ANSI((CELLPTR, int, int));
char *Expand ANSI((char *));
char *Apply_edit ANSI((char *, char *, char *, int, int));
void Map_esc ANSI((char *));
char* Apply_modifiers ANSI((int, char *));
char* Tokenize ANSI((char *, char *, char, int));
char* ScanToken ANSI((char *, char **, int));
char *DmStrJoin ANSI((char *, char *, int, int));
char *DmStrAdd ANSI((char *, char *, int));
char *DmStrApp ANSI((char *, char *));
char *DmStrDup ANSI((char *));
char *DmStrDup2 ANSI((char *));
char *DmStrPbrk ANSI((char *, char *));
char *DmStrSpn ANSI((char *, char *));
char *DmStrStr ANSI((char *, char *));
char *DmSubStr ANSI((char *, char *));
uint16 Hash ANSI((char *, uint32 *));
HASHPTR Get_name ANSI((char *, HASHPTR *, int));
HASHPTR Search_table ANSI((HASHPTR *, char *, uint16 *, uint32 *));
HASHPTR Push_macro ANSI((HASHPTR));
HASHPTR Pop_macro ANSI((HASHPTR));
HASHPTR Def_macro ANSI((char *, char *, int));
CELLPTR Def_cell ANSI((char *));
LINKPTR Add_prerequisite ANSI((CELLPTR, CELLPTR, int, int));
void Clear_prerequisites ANSI((CELLPTR));
int Test_circle ANSI((CELLPTR, int));
STRINGPTR Def_recipe ANSI((char *, STRINGPTR, int, int));
t_attr Rcp_attribute ANSI((char *));
int main ANSI((int, char **));
FILE *Openfile ANSI((char *, int, int));
FILE *Closefile ANSI(());
FILE *Search_file ANSI((char *, char **));
char *Filename ANSI(());
int Nestlevel ANSI(());
FILE *TryFiles ANSI((LINKPTR));
void Fatal ANSI((ARG (char *,fmt),ARG (va_alist_type, va_alist)));
void Error ANSI((ARG (char *,fmt),ARG (va_alist_type, va_alist)));
void Warning ANSI((ARG (char *,fmt),ARG (va_alist_type, va_alist)));
void No_ram ANSI(());
void Usage ANSI((int));
void Version ANSI(());
char *Get_suffix ANSI((char *));
char *Basename ANSI((char *));
char *Filedir ANSI((char *));
char *Build_path ANSI((char *, char *));
void Make_rules ANSI(());
void Create_macro_vars ANSI(());
time_t Do_stat ANSI((char *, char *, char **, int));
int Do_touch ANSI((char *, char *, char **));
void Void_lib_cache ANSI((char *, char *));
time_t Do_time ANSI(());
int Do_cmnd ANSI((char *, int, int, CELLPTR, int, int, int));
char ** Pack_argv ANSI((int, int, char *));
char *Read_env_string ANSI((char *));
int Write_env_string ANSI((char *, char *));
void ReadEnvironment ANSI(());
void Catch_signals ANSI((void (*)()));
void Clear_signals ANSI(());
void Prolog ANSI((int, char* []));
void Epilog ANSI((int));
char *Get_current_dir ANSI(());
int Set_dir ANSI((char*));
char Get_switch_char ANSI(());
FILE* Get_temp ANSI((char **, char *, int));
FILE *Start_temp ANSI((char *, CELLPTR, char **));
void Open_temp_error ANSI((char *, char *));
void Link_temp ANSI((CELLPTR, FILE *, char *));
void Close_temp ANSI((CELLPTR, FILE *));
void Unlink_temp_files ANSI((CELLPTR));
void Handle_result ANSI((int, int, int, CELLPTR));
void Update_time_stamp ANSI((CELLPTR));
int Remove_file ANSI((char *));
void Parse ANSI((FILE *));
int Get_line ANSI((char *, FILE *));
char *Do_comment ANSI((char *, char **, int));
char *Get_token ANSI((TKSTRPTR, char *, int));
void Quit ANSI(());
void Read_state ANSI(());
void Write_state ANSI(());
int Check_state ANSI((CELLPTR, STRINGPTR *, int));
void Dump ANSI(());
void Dump_recipe ANSI((STRINGPTR));
int Parse_macro ANSI((char *, int));
int Macro_op ANSI((char *));
int Parse_rule_def ANSI((int *));
int Rule_op ANSI((char *));
void Add_recipe_to_list ANSI((char *, int, int));
void Bind_rules_to_targets ANSI((int));
int Set_group_attributes ANSI((char *));
DFALINKPTR Match_dfa ANSI((char *));
void Check_circle_dfa ANSI(());
void Add_nfa ANSI((char *));
char *Exec_function ANSI((char *));
time_t seek_arch ANSI((char *, char *));
int If_root_path ANSI((char *));
void Remove_prq ANSI((CELLPTR));
int runargv ANSI((CELLPTR, int, int, int, int, char *));
int Wait_for_child ANSI((int, int));
void Clean_up_processes ANSI(());
time_t CacheStat ANSI((char *, int));

#endif
