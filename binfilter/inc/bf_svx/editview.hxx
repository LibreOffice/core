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

// MyEDITVIEW, wegen exportiertem EditView
#ifndef _MyEDITVIEW_HXX
#define _MyEDITVIEW_HXX

#ifndef _RSCSFX_HXX //autogen
#include <rsc/rscsfx.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
class Window;
class Pointer;
class Cursor;
class KeyEvent;
class MouseEvent;
class Rectangle;
class Link;
class Pair;
class Point;
class Range;
class SvStream;
namespace binfilter {
class SvKeyValueIterator;

class SfxStyleSheet; 
class EditEngine;
class ImpEditEngine;
class ImpEditView;
class SvxFieldItem;
class DropEvent;
class CommandEvent;
}//end of namespace binfilter //namespace binfilter
#ifndef _EDITDATA_HXX
#include <bf_svx/editdata.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com {
namespace sun {
namespace star {
namespace datatransfer {
    class XTransferable;
}}}}

namespace binfilter {
class EditView
{
    friend class EditEngine;
    friend class ImpEditEngine;
    friend class EditSelFunctionSet;

public:	// brauche ich fuer Undo
    ImpEditView*	GetImpEditView() const 		{ return pImpEditView; }

private:
    ImpEditView*	pImpEditView;

                    EditView( const EditView& );

public:
    virtual			~EditView();

    EditEngine*		GetEditEngine() const;

    Window*			GetWindow() const;


    void			ShowCursor( BOOL bGotoCursor = TRUE, BOOL bForceVisCursor = TRUE );
    void			HideCursor();

    BOOL			HasSelection() const;
    ESelection		GetSelection() const;
    void			SetSelection( const ESelection& rNewSel );


    const Rectangle&	GetOutputArea() const;

    const Rectangle&	GetVisArea() const;

    void			Cut();
    void			Copy();
    void			Paste();
};

}//end of namespace binfilter
#endif // _MyEDITVIEW_HXX
