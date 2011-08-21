/*
 * (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
/************************************************************************/
/*                                  */
/* this include sets up some macro functions which can be used while    */
/* debugging the program, and then left in the code, but turned of by   */
/* just not defining "DEBUG".  This way your production version of  */
/* the program will not be filled with bunches of debugging junk    */
/*                                  */
/************************************************************************/
/*
 * $Id: debug.h,v 1.2 2006-07-25 10:07:24 rt Exp $
 */

#ifdef DEBUG

#if DEBUG == 1          /* if default level         */
#undef DEBUG
#define DEBUG   100     /*   use level 100          */
#endif

#include <stdio.h>

#define DEBUG0(val,str)\
                {\
                  if( DEBUG > val ) \
                    fprintf(stderr,"%s(%d): %s\n",\
                        __FILE__,__LINE__,str);\
                }
#define DEBUG1(val,str,a1)\
                    {\
                  char _debugbuf[100];\
                  if( DEBUG > val )\
                   {\
                    sprintf(_debugbuf,str,a1);\
                    fprintf(stderr,"%s(%d): %s\n",\
                        __FILE__,__LINE__,_debugbuf);\
                   }\
                       }

#define DEBUG2(val,str,a1,a2)\
                    {\
                 char _debugbuf[100];\
                  if( DEBUG > val )\
                   {\
                    sprintf(_debugbuf,str,a1,a2);\
                    fprintf(stderr,"%s(%d): %s\n",\
                        __FILE__,__LINE__,_debugbuf);\
                   }\
                       }

#define DEBUG3(val,str,a1,a2,a3)\
                    {\
                  char _debugbuf[100];\
                  if( DEBUG > val )\
                   {\
                    sprintf(_debugbuf,str,a1,a2,a3);\
                    fprintf(stderr,"%s(%d): %s\n",\
                        __FILE__,__LINE__,_debugbuf);\
                   }\
                       }

#define DEBUG4(val,str,a1,a2,a3,a4)\
                     {\
                  char _debugbuf[100];\
                  if( DEBUG > val )\
                   {\
                    sprintf(_debugbuf,str,a1,a2,a3,a4);\
                    fprintf(stderr,"%s(%d): %s\n",\
                        __FILE__,__LINE__,_debugbuf);\
                   }\
                       }

#define DEBUG5(val,str,a1,a2,a3,a4,a5)\
                     {\
                  char _debugbuf[100];\
                  if( DEBUG > val )\
                   {\
                    sprintf(_debugbuf,str,a1,a2,a3,a4,a5);\
                    fprintf(stderr,"%s(%d): %s\n",\
                        __FILE__,__LINE__,_debugbuf);\
                   }\
                       }

#else

#define DEBUG0(val,s)
#define DEBUG1(val,s,a1)
#define DEBUG2(val,s,a1,a2)
#define DEBUG3(val,s,a1,a2,a3)
#define DEBUG4(val,s,a1,a2,a3,a4)
#define DEBUG5(val,s,a1,a2,a3,a4,a5)

#endif /* DEBUG */


