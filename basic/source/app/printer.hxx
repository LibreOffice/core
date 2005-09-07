/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:18:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BASICPRN_HXX
#define _BASICPRN_HXX

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif

class PrintingDialog;

class BasicPrinter : public Printer {
    short nLine;                        // aktuelle Zeile
    short nPage;                        // aktuelle Seite
    short nLines;                       // Zeilen pro Seite
    short nYoff;                        // Zeilenabstand in Points
    String aFile;                       // Dateiname
    PrintingDialog* pDlg;               // Druck-Dialog
    void Header();                      // Seitenkopf drucken
public:
    BasicPrinter();
    void Print( const String& rFile, const String& rText, BasicFrame *pFrame );
    DECL_LINK( Abort, void * );
    DECL_LINK( StartPrintHdl, Printer * );
    DECL_LINK( EndPrintHdl, Printer * );
    DECL_LINK( PrintPageHdl, Printer * );
#if defined( PM20 )
    // StarView-Bug, bald wieder zu entfernen:
    virtual void SetPageQueueSize( USHORT ) {}
#endif
};

#endif
