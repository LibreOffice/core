/*************************************************************************
 *
 *  $RCSfile: gluepts.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-31 12:12:52 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_GLUEPOINT2_HDL_
#include <com/sun/star/drawing/GluePoint2.hpp>
#endif

#include <cppuhelper/implbase1.hxx>

#include "svdmodel.hxx"
#include "svdobj.hxx"
#include "svdglue.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

class SvxUnoGluePointAccess : public WeakImplHelper1< container::XIndexContainer >,
                                public SfxListener
{
private:
    SdrObject*  mpObject;

public:
    SvxUnoGluePointAccess( SdrObject* pObject ) throw();
    virtual ~SvxUnoGluePointAccess() throw();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(uno::RuntimeException);
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException);
};

static void convert( const SdrGluePoint& rSdrGlue, drawing::GluePoint2& rUnoGlue ) throw()
{
    rUnoGlue.Position.X = rSdrGlue.GetPos().X();
    rUnoGlue.Position.Y = rSdrGlue.GetPos().Y();
    rUnoGlue.IsRelative = rSdrGlue.IsPercent();

    switch( rSdrGlue.GetAlign() )
    {
    case SDRVERTALIGN_TOP|SDRHORZALIGN_LEFT:
        rUnoGlue.PositionAlignment = drawing::Alignment_TOP_LEFT;
        break;
    case SDRHORZALIGN_CENTER|SDRVERTALIGN_TOP:
        rUnoGlue.PositionAlignment = drawing::Alignment_TOP;
        break;
    case SDRVERTALIGN_TOP|SDRHORZALIGN_RIGHT:
        rUnoGlue.PositionAlignment = drawing::Alignment_TOP_RIGHT;
        break;
    case SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER:
        rUnoGlue.PositionAlignment = drawing::Alignment_CENTER;
        break;
    case SDRHORZALIGN_RIGHT|SDRVERTALIGN_CENTER:
        rUnoGlue.PositionAlignment = drawing::Alignment_RIGHT;
        break;
    case SDRHORZALIGN_LEFT|SDRVERTALIGN_BOTTOM:
        rUnoGlue.PositionAlignment = drawing::Alignment_BOTTOM_LEFT;
        break;
    case SDRHORZALIGN_CENTER|SDRVERTALIGN_BOTTOM:
        rUnoGlue.PositionAlignment = drawing::Alignment_BOTTOM;
        break;
    case SDRHORZALIGN_RIGHT|SDRVERTALIGN_BOTTOM:
        rUnoGlue.PositionAlignment = drawing::Alignment_BOTTOM_RIGHT;
        break;
//  case SDRHORZALIGN_LEFT:
    default:
        rUnoGlue.PositionAlignment = drawing::Alignment_LEFT;
        break;
    }

    switch( rSdrGlue.GetEscDir() )
    {
    case SDRESC_LEFT:
        rUnoGlue.Escape = drawing::EscapeDirection_LEFT;
        break;
    case SDRESC_RIGHT:
        rUnoGlue.Escape = drawing::EscapeDirection_RIGHT;
        break;
    case SDRESC_TOP:
        rUnoGlue.Escape = drawing::EscapeDirection_UP;
        break;
    case SDRESC_BOTTOM:
        rUnoGlue.Escape = drawing::EscapeDirection_DOWN;
        break;
    case SDRESC_HORZ:
        rUnoGlue.Escape = drawing::EscapeDirection_HORIZONTAL;
        break;
    case SDRESC_VERT:
        rUnoGlue.Escape = drawing::EscapeDirection_VERTICAL;
        break;
//          case SDRESC_SMART:
    default:
        rUnoGlue.Escape = drawing::EscapeDirection_SMART;
        break;
    }
}

static void convert( const drawing::GluePoint2& rUnoGlue, SdrGluePoint& rSdrGlue ) throw()
{
    rSdrGlue.SetPos( Point( rUnoGlue.Position.X, rUnoGlue.Position.Y ) );
    rSdrGlue.SetPercent( rUnoGlue.IsRelative );

    switch( rUnoGlue.PositionAlignment )
    {
    case drawing::Alignment_TOP_LEFT:
        rSdrGlue.SetAlign( SDRVERTALIGN_TOP|SDRHORZALIGN_LEFT );
        break;
    case drawing::Alignment_TOP:
        rSdrGlue.SetAlign( SDRHORZALIGN_CENTER|SDRVERTALIGN_TOP );
        break;
    case drawing::Alignment_TOP_RIGHT:
        rSdrGlue.SetAlign( SDRVERTALIGN_TOP|SDRHORZALIGN_RIGHT );
        break;
    case drawing::Alignment_CENTER:
        rSdrGlue.SetAlign( SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER );
        break;
    case drawing::Alignment_RIGHT:
        rSdrGlue.SetAlign( SDRHORZALIGN_RIGHT|SDRVERTALIGN_CENTER );
        break;
    case SDRHORZALIGN_LEFT|SDRVERTALIGN_BOTTOM:
        rSdrGlue.SetAlign( SDRHORZALIGN_LEFT|SDRVERTALIGN_BOTTOM );
        break;
    case drawing::Alignment_BOTTOM:
        rSdrGlue.SetAlign( SDRHORZALIGN_CENTER|SDRVERTALIGN_BOTTOM );
        break;
    case drawing::Alignment_BOTTOM_RIGHT:
        rSdrGlue.SetAlign( SDRHORZALIGN_RIGHT|SDRVERTALIGN_BOTTOM );
        break;
//  case SDRHORZALIGN_LEFT:
    default:
        rSdrGlue.SetAlign( SDRHORZALIGN_LEFT );
        break;
    }
    switch( rUnoGlue.Escape )
    {
    case drawing::EscapeDirection_LEFT:
        rSdrGlue.SetEscDir(SDRESC_LEFT);
        break;
    case drawing::EscapeDirection_RIGHT:
        rSdrGlue.SetEscDir(SDRESC_RIGHT);
        break;
    case drawing::EscapeDirection_UP:
        rSdrGlue.SetEscDir(SDRESC_TOP);
        break;
    case drawing::EscapeDirection_DOWN:
        rSdrGlue.SetEscDir(SDRESC_BOTTOM);
        break;
    case drawing::EscapeDirection_HORIZONTAL:
        rSdrGlue.SetEscDir(SDRESC_HORZ);
        break;
    case drawing::EscapeDirection_VERTICAL:
        rSdrGlue.SetEscDir(SDRESC_VERT);
        break;
//  case drawing::EscapeDirection_SMART:
    default:
        rSdrGlue.SetEscDir(SDRESC_SMART);
        break;
    }
}

SvxUnoGluePointAccess::SvxUnoGluePointAccess( SdrObject* pObject ) throw()
: mpObject( pObject )
{
    StartListening( *mpObject->GetModel() );

}

SvxUnoGluePointAccess::~SvxUnoGluePointAccess() throw()
{
    if( mpObject && mpObject->GetModel())
        EndListening( *mpObject->GetModel() );
}

void SvxUnoGluePointAccess::Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw()
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint && mpObject)
    {
        if( pSdrHint->GetKind() == HINT_OBJREMOVED )
        {
            if( mpObject == pSdrHint->GetObject() )
                mpObject = NULL;
        }
        else if( pSdrHint->GetKind() == HINT_MODELCLEARED || pSdrHint->GetKind() == HINT_OBJLISTCLEARED )
        {
            mpObject = NULL;
        }
    }
}

// XIndexContainer
void SAL_CALL SvxUnoGluePointAccess::insertByIndex( sal_Int32 Index, const uno::Any& Element )
    throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpObject )
    {
        SdrGluePointList* pList = mpObject->ForceGluePointList();
        if( pList )
        {
            SdrGluePoint aSdrGlue;
            drawing::GluePoint2 aUnoGlue;

            if( Element >>= aUnoGlue )
            {
                convert( aUnoGlue, aSdrGlue );
                pList->Insert( aSdrGlue );
                mpObject->SendRepaintBroadcast();
                return;
            }

            throw lang::IllegalArgumentException();
        }
    }

    throw lang::IndexOutOfBoundsException();
}

void SAL_CALL SvxUnoGluePointAccess::removeByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpObject )
    {
        SdrGluePointList* pList = mpObject->ForceGluePointList();
        if( pList )
        {
            Index -= 4;
            if( Index >= 0 && Index < pList->GetCount() )
            {
                pList->Delete( (USHORT)Index );
                mpObject->SendRepaintBroadcast();
                return;
            }
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XIndexReplace
void SAL_CALL SvxUnoGluePointAccess::replaceByIndex( sal_Int32 Index, const uno::Any& Element )
    throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException,
    uno::RuntimeException)
{
    drawing::GluePoint2 aUnoGlue;
    if(!(Element >>= aUnoGlue))
        throw lang::IllegalArgumentException();

    Index -= 4;
    if( mpObject && Index >= 0 )
    {
        SdrGluePointList* pList = mpObject->GetGluePointList();
        if( pList && Index < pList->GetCount() )
        {
            SdrGluePoint& rGlue = (*pList)[(USHORT)Index];
            convert( aUnoGlue, rGlue );
            mpObject->SendRepaintBroadcast();
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XIndexAccess
sal_Int32 SAL_CALL SvxUnoGluePointAccess::getCount()
    throw(uno::RuntimeException)
{
    sal_Int32 nCount = 0;
    if( mpObject )
    {
        // each node has a default of 4 glue points
        // and any number of user defined glue points
        if( mpObject->IsNode() )
        {
            nCount += 4;

            const SdrGluePointList* pList = mpObject->GetGluePointList();
            nCount += pList->GetCount();
        }
    }

    return nCount;
}

uno::Any SAL_CALL SvxUnoGluePointAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( Index > 0 && mpObject && mpObject->IsNode() )
    {
        struct drawing::GluePoint2 aGluePoint;

        if( Index < 4 ) // default glue point?
        {
            SdrGluePoint aTempPoint = mpObject->GetVertexGluePoint( (USHORT)Index );
            aGluePoint.IsUserDefined = sal_True;
            convert( aTempPoint, aGluePoint );
            uno::Any aAny;
            aAny <<= aGluePoint;
            return aAny;
        }
        else
        {
            Index -= 4;
            const SdrGluePointList* pList = mpObject->GetGluePointList();
            if( pList && Index < pList->GetCount() )
            {
                const SdrGluePoint& rTempPoint = (*pList)[(USHORT)Index];
                aGluePoint.IsUserDefined = sal_False;
                convert( rTempPoint, aGluePoint );
                uno::Any aAny;
                aAny <<= aGluePoint;
                return aAny;
            }
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoGluePointAccess::getElementType()
    throw( uno::RuntimeException)
{
    return ::getCppuType((const struct drawing::GluePoint2*)0);
}

sal_Bool SAL_CALL SvxUnoGluePointAccess::hasElements()
    throw( uno::RuntimeException)
{
    return mpObject && mpObject->IsNode();
}

/**
 * Create a SvxUnoGluePointAccess
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoGluePointAccess_createInstance( SdrObject* pObject )
{
    return *new SvxUnoGluePointAccess(pObject);
}
