/*************************************************************************
 *
 *  $RCSfile: reffact.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-09 19:57:03 $
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
DECL_WRAPPER(ScPivotLayoutWrapper)
DECL_WRAPPER(ScTabOpDlgWrapper)
DECL_WRAPPER(ScFilterDlgWrapper)
DECL_WRAPPER(ScSpecialFilterDlgWrapper)
DECL_WRAPPER(ScDbNameDlgWrapper)
DECL_WRAPPER(ScConsolidateDlgWrapper)
DECL_WRAPPER(ScChartDlgWrapper)
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
        void            SetFlags( BOOL bCloseOnButtonUp );
        static void     SetAutoReOpen(BOOL bFlag);

        void            StartRefInput();
};


//==================================================================


#endif // SC_REFFACT_HXX
