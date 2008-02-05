/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reffact.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-05 15:46:14 $
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

#ifndef SC_REFFACT_HXX
#define SC_REFFACT_HXX

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#include "dbfunc.hxx"

#define DECL_WRAPPER(Class) \
    class Class : public SfxChildWindow                                         \
    {                                                                           \
    public:                                                                     \
        Class( Window*, USHORT, SfxBindings*, SfxChildWinInfo* );               \
        SFX_DECL_CHILDWINDOW(Class);                                            \
    };


//==================================================================

DECL_WRAPPER(ScNameDlgWrapper)
DECL_WRAPPER(ScSolverDlgWrapper)
DECL_WRAPPER(ScOptSolverDlgWrapper)
DECL_WRAPPER(ScPivotLayoutWrapper)
DECL_WRAPPER(ScTabOpDlgWrapper)
DECL_WRAPPER(ScFilterDlgWrapper)
DECL_WRAPPER(ScSpecialFilterDlgWrapper)
DECL_WRAPPER(ScDbNameDlgWrapper)
DECL_WRAPPER(ScConsolidateDlgWrapper)
DECL_WRAPPER(ScPrintAreasDlgWrapper)
DECL_WRAPPER(ScCondFormatDlgWrapper)
DECL_WRAPPER(ScColRowNameRangesDlgWrapper)
DECL_WRAPPER(ScFormulaDlgWrapper)
DECL_WRAPPER(ScHighlightChgDlgWrapper)

/*!!! dafuer muss der Funktionsautopilot noch umgebaut werden
DECL_WRAPPER(ScFunctionDlgWrapper)
DECL_WRAPPER(ScEditFunctionDlgWrapper)
DECL_WRAPPER(ScArgumentDlgWrapper)
*/

class ScAcceptChgDlgWrapper: public SfxChildWindow
{
    public:
        ScAcceptChgDlgWrapper(  Window*,
                                USHORT,
                                SfxBindings*,
                                SfxChildWinInfo* );

        SFX_DECL_CHILDWINDOW(Class);

        virtual void ReInitDlg();
};

class ScSimpleRefDlgWrapper: public SfxChildWindow
{
    public:
        ScSimpleRefDlgWrapper(  Window*,
                                USHORT,
                                SfxBindings*,
                                SfxChildWinInfo* );

        SFX_DECL_CHILDWINDOW(Class);

        static void     SetDefaultPosSize(Point aPos, Size aSize, BOOL bSet=TRUE);
        virtual String  GetRefString();
        virtual void    SetRefString(const String& rStr);
        void            SetCloseHdl( const Link& rLink );
        void            SetUnoLinks( const Link& rDone, const Link& rAbort,
                                        const Link& rChange );
        void            SetFlags( BOOL bCloseOnButtonUp, BOOL bSingleCell, BOOL bMultiSelection );
        static void     SetAutoReOpen(BOOL bFlag);

        void            StartRefInput();
};


//==================================================================


#endif // SC_REFFACT_HXX
