/*************************************************************************
#*
#*    $Workfile:   svpm.h  $
#*
#*    Ersterstellung    DV 01.10.97
#*    Letzte Aenderung  $Author: pluby $ $Date: 2000-11-28 19:13:56 $
#*    $Revision: 1.3 $
#*
#*    $Logfile:   T:/tools/os2/inc/svpm.h_v  $
#*
#*    Copyright (c) 2000, Sun Microsystems, Inc.
#*
#*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/solenv/inc/premac.h,v 1.3 2000-11-28 19:13:56 pluby Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/09/27 22:52:33  patrick.luby
      Updates to handle <AppKit/AppKit.h> includes

      Revision 1.1.1.1  2000/09/20 14:43:16  hr
      initial import

      Revision 1.2  2000/06/26 17:08:58  patrick.luby
      Updated files so that the %minor.mk dmake recipe will work for developers who
      must use rsync to get the solver directories.

      Revision 1.1  2000/06/11 09:18:58  patrick.luby
      Updates to support SRC591 build on Mac OS X.

      Revision 1.2  1997/10/15 12:26:32  dv
      MacOSPtr und MacOSBoolean neu

      Revision 1.1  1997/10/01 06:45:41  dv
      First checked in


*************************************************************************/

#define Boolean MacOSBoolean
#define Button MacOSButton
#define Byte MacOSByte
#define Control MacOSControl
#define Cursor MacOSCursor
#define FontInfo MacOSFontInfo
#define MemoryBlock MacOSMemoryBlock
#define Point MacOSPoint
#define Size MacOSSize
#define Region MacOSRegion
#define Polygon MacOSPolygon
#define Ptr MacOSPtr
#define Palette MacOSPalette
#define LSize MacOSLSize
#define ModalDialog MacOSModalDialog
#define SetCursor MacOSSetCursor
#define ShowWindow MacOSShowWindow
#define StringPtr MacOSStringPtr
#define DirInfo MacOSDirInfo
#define BOOL MacOSBOOL
#define ULONG MacOSULONG
#define Line MacOSLine
