/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>

#include <cppuhelper/implbase.hxx>

#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdpage.hxx>

#include <gluepts.hxx>

using namespace ::com::sun::star;
using namespace ::cppu;

const sal_uInt16 NON_USER_DEFINED_GLUE_POINTS = 4;

class SvxUnoGluePointAccess : public WeakImplHelper< container::XIndexContainer, container::XIdentifierContainer >
{
private:
    SdrObjectWeakRef    mpObject;

public:
    explicit SvxUnoGluePointAccess( SdrObject* pObject ) throw();
    virtual ~SvxUnoGluePointAccess() throw();

    // XIdentifierContainer
    virtual sal_Int32 SAL_CALL insert( const uno::Any& aElement ) throw (lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    // XIdentifierReplace
    virtual void SAL_CALL replaceByIdentifer( sal_Int32 Identifier, const uno::Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    // XIdentifierReplace
    virtual uno::Any SAL_CALL getByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;
    virtual uno::Sequence< sal_Int32 > SAL_CALL getIdentifiers(  ) throw (uno::RuntimeException, std::exception) override;

    /* deprecated */
    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    /* deprecated */
    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    /* deprecated */
    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(uno::RuntimeException, std::exception) override;
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException, std::exception) override;
};

static void convert( const SdrGluePoint& rSdrGlue, drawing::GluePoint2& rUnoGlue ) throw()
{
    rUnoGlue.Position.X = rSdrGlue.GetPos().X();
    rUnoGlue.Position.Y = rSdrGlue.GetPos().Y();
    rUnoGlue.IsRelative = rSdrGlue.IsPercent();

    SdrAlign eAlign = rSdrGlue.GetAlign();
    if (eAlign == (SdrAlign::VERT_TOP|SdrAlign::HORZ_LEFT))
        rUnoGlue.PositionAlignment = drawing::Alignment_TOP_LEFT;
    else if (eAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_TOP))
        rUnoGlue.PositionAlignment = drawing::Alignment_TOP;
    else if (eAlign == (SdrAlign::VERT_TOP|SdrAlign::HORZ_RIGHT))
        rUnoGlue.PositionAlignment = drawing::Alignment_TOP_RIGHT;
    else if (eAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_CENTER))
        rUnoGlue.PositionAlignment = drawing::Alignment_CENTER;
    else if (eAlign == (SdrAlign::HORZ_RIGHT|SdrAlign::VERT_CENTER))
        rUnoGlue.PositionAlignment = drawing::Alignment_RIGHT;
    else if (eAlign == (SdrAlign::HORZ_LEFT|SdrAlign::VERT_BOTTOM))
        rUnoGlue.PositionAlignment = drawing::Alignment_BOTTOM_LEFT;
    else if (eAlign == (SdrAlign::HORZ_CENTER|SdrAlign::VERT_BOTTOM))
        rUnoGlue.PositionAlignment = drawing::Alignment_BOTTOM;
    else if (eAlign == (SdrAlign::HORZ_RIGHT|SdrAlign::VERT_BOTTOM))
        rUnoGlue.PositionAlignment = drawing::Alignment_BOTTOM_RIGHT;
    else {
        rUnoGlue.PositionAlignment = drawing::Alignment_LEFT;
    }

    switch( rSdrGlue.GetEscDir() )
    {
    case SdrEscapeDirection::LEFT:
        rUnoGlue.Escape = drawing::EscapeDirection_LEFT;
        break;
    case SdrEscapeDirection::RIGHT:
        rUnoGlue.Escape = drawing::EscapeDirection_RIGHT;
        break;
    case SdrEscapeDirection::TOP:
        rUnoGlue.Escape = drawing::EscapeDirection_UP;
        break;
    case SdrEscapeDirection::BOTTOM:
        rUnoGlue.Escape = drawing::EscapeDirection_DOWN;
        break;
    case SdrEscapeDirection::HORZ:
        rUnoGlue.Escape = drawing::EscapeDirection_HORIZONTAL;
        break;
    case SdrEscapeDirection::VERT:
        rUnoGlue.Escape = drawing::EscapeDirection_VERTICAL;
        break;
//          case SdrEscapeDirection::SMART:
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
        rSdrGlue.SetAlign( SdrAlign::VERT_TOP|SdrAlign::HORZ_LEFT );
        break;
    case drawing::Alignment_TOP:
        rSdrGlue.SetAlign( SdrAlign::HORZ_CENTER|SdrAlign::VERT_TOP );
        break;
    case drawing::Alignment_TOP_RIGHT:
        rSdrGlue.SetAlign( SdrAlign::VERT_TOP|SdrAlign::HORZ_RIGHT );
        break;
    case drawing::Alignment_CENTER:
        rSdrGlue.SetAlign( SdrAlign::HORZ_CENTER|SdrAlign::VERT_CENTER );
        break;
    case drawing::Alignment_RIGHT:
        rSdrGlue.SetAlign( SdrAlign::HORZ_RIGHT|SdrAlign::VERT_CENTER );
        break;
    case drawing::Alignment_BOTTOM_LEFT:
        rSdrGlue.SetAlign( SdrAlign::HORZ_LEFT|SdrAlign::VERT_BOTTOM );
        break;
    case drawing::Alignment_BOTTOM:
        rSdrGlue.SetAlign( SdrAlign::HORZ_CENTER|SdrAlign::VERT_BOTTOM );
        break;
    case drawing::Alignment_BOTTOM_RIGHT:
        rSdrGlue.SetAlign( SdrAlign::HORZ_RIGHT|SdrAlign::VERT_BOTTOM );
        break;
//  case SdrAlign::HORZ_LEFT:
    default:
        rSdrGlue.SetAlign( SdrAlign::HORZ_LEFT );
        break;
    }
    switch( rUnoGlue.Escape )
    {
    case drawing::EscapeDirection_LEFT:
        rSdrGlue.SetEscDir(SdrEscapeDirection::LEFT);
        break;
    case drawing::EscapeDirection_RIGHT:
        rSdrGlue.SetEscDir(SdrEscapeDirection::RIGHT);
        break;
    case drawing::EscapeDirection_UP:
        rSdrGlue.SetEscDir(SdrEscapeDirection::TOP);
        break;
    case drawing::EscapeDirection_DOWN:
        rSdrGlue.SetEscDir(SdrEscapeDirection::BOTTOM);
        break;
    case drawing::EscapeDirection_HORIZONTAL:
        rSdrGlue.SetEscDir(SdrEscapeDirection::HORZ);
        break;
    case drawing::EscapeDirection_VERTICAL:
        rSdrGlue.SetEscDir(SdrEscapeDirection::VERT);
        break;
//  case drawing::EscapeDirection_SMART:
    default:
        rSdrGlue.SetEscDir(SdrEscapeDirection::SMART);
        break;
    }
}

SvxUnoGluePointAccess::SvxUnoGluePointAccess( SdrObject* pObject ) throw()
: mpObject( pObject )
{
}

SvxUnoGluePointAccess::~SvxUnoGluePointAccess() throw()
{
}

// XIdentifierContainer
sal_Int32 SAL_CALL SvxUnoGluePointAccess::insert( const uno::Any& aElement ) throw (lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpObject.is() )
    {
        SdrGluePointList* pList = mpObject->ForceGluePointList();
        if( pList )
        {
            // second, insert the new glue point
            drawing::GluePoint2 aUnoGlue;

            if( aElement >>= aUnoGlue )
            {
                SdrGluePoint aSdrGlue;
                convert( aUnoGlue, aSdrGlue );
                sal_uInt16 nId = pList->Insert( aSdrGlue );

                // only repaint, no objectchange
                mpObject->ActionChanged();
                // mpObject->BroadcastObjectChange();

                return (sal_Int32)((*pList)[nId].GetId() + NON_USER_DEFINED_GLUE_POINTS) - 1;
            }

            throw lang::IllegalArgumentException();
        }
    }

    return -1;
}

void SAL_CALL SvxUnoGluePointAccess::removeByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpObject.is() && ( Identifier >= NON_USER_DEFINED_GLUE_POINTS ))
    {
        const sal_uInt16 nId = (sal_uInt16)(Identifier - NON_USER_DEFINED_GLUE_POINTS) + 1;

        SdrGluePointList* pList = const_cast<SdrGluePointList*>(mpObject->GetGluePointList());
        const sal_uInt16 nCount = pList ? pList->GetCount() : 0;
        sal_uInt16 i;

        for( i = 0; i < nCount; i++ )
        {
            if( (*pList)[i].GetId() == nId )
            {
                pList->Delete( i );

                // only repaint, no objectchange
                mpObject->ActionChanged();
                // mpObject->BroadcastObjectChange();

                return;
            }
        }
    }

    throw container::NoSuchElementException();
}

// XIdentifierReplace
void SAL_CALL SvxUnoGluePointAccess::replaceByIdentifer( sal_Int32 Identifier, const uno::Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpObject.is() && mpObject->IsNode() )
    {
        struct drawing::GluePoint2 aGluePoint;
        if( (Identifier < NON_USER_DEFINED_GLUE_POINTS) || !(aElement >>= aGluePoint))
            throw lang::IllegalArgumentException();

        const sal_uInt16 nId = (sal_uInt16)( Identifier - NON_USER_DEFINED_GLUE_POINTS ) + 1;

        SdrGluePointList* pList = const_cast< SdrGluePointList* >( mpObject->GetGluePointList() );
        const sal_uInt16 nCount = pList ? pList->GetCount() : 0;
        sal_uInt16 i;
        for( i = 0; i < nCount; i++ )
        {
            if( (*pList)[i].GetId() == nId )
            {
                // change the glue point
                SdrGluePoint& rTempPoint = (*pList)[i];
                convert( aGluePoint, rTempPoint );

                // only repaint, no objectchange
                mpObject->ActionChanged();
                // mpObject->BroadcastObjectChange();

                return;
            }
        }

        throw container::NoSuchElementException();
    }
}

// XIdentifierAccess
uno::Any SAL_CALL SvxUnoGluePointAccess::getByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpObject.is() && mpObject->IsNode() )
    {
        struct drawing::GluePoint2 aGluePoint;

        if( Identifier < NON_USER_DEFINED_GLUE_POINTS ) // default glue point?
        {
            SdrGluePoint aTempPoint = mpObject->GetVertexGluePoint( (sal_uInt16)Identifier );
            aGluePoint.IsUserDefined = sal_False;
            convert( aTempPoint, aGluePoint );
            return uno::makeAny( aGluePoint );
        }
        else
        {
            const sal_uInt16 nId = (sal_uInt16)( Identifier - NON_USER_DEFINED_GLUE_POINTS ) + 1;

            const SdrGluePointList* pList = mpObject->GetGluePointList();
            const sal_uInt16 nCount = pList ? pList->GetCount() : 0;
            for( sal_uInt16 i = 0; i < nCount; i++ )
            {
                const SdrGluePoint& rTempPoint = (*pList)[i];
                if( rTempPoint.GetId() == nId )
                {
                    // #i38892#
                    if(rTempPoint.IsUserDefined())
                    {
                        aGluePoint.IsUserDefined = sal_True;
                    }

                    convert( rTempPoint, aGluePoint );
                    return uno::makeAny( aGluePoint );
                }
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< sal_Int32 > SAL_CALL SvxUnoGluePointAccess::getIdentifiers() throw (uno::RuntimeException, std::exception)
{
    if( mpObject.is() )
    {
        const SdrGluePointList* pList = mpObject->GetGluePointList();
        const sal_uInt16 nCount = pList ? pList->GetCount() : 0;

        sal_uInt16 i;

        uno::Sequence< sal_Int32 > aIdSequence( nCount + NON_USER_DEFINED_GLUE_POINTS );
        sal_Int32 *pIdentifier = aIdSequence.getArray();

        for( i = 0; i < NON_USER_DEFINED_GLUE_POINTS; i++ )
            *pIdentifier++ = (sal_Int32)i;

        for( i = 0; i < nCount; i++ )
            *pIdentifier++ = (sal_Int32) ( (*pList)[i].GetId() + NON_USER_DEFINED_GLUE_POINTS ) - 1;

        return aIdSequence;
    }
    else
    {
        uno::Sequence< sal_Int32 > aEmpty;
        return aEmpty;
    }
}

/* deprecated */

// XIndexContainer
void SAL_CALL SvxUnoGluePointAccess::insertByIndex( sal_Int32, const uno::Any& Element )
    throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException,
            lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpObject.is() )
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

                // only repaint, no objectchange
                mpObject->ActionChanged();
                // mpObject->BroadcastObjectChange();

                return;
            }

            throw lang::IllegalArgumentException();
        }
    }

    throw lang::IndexOutOfBoundsException();
}

void SAL_CALL SvxUnoGluePointAccess::removeByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpObject.is() )
    {
        SdrGluePointList* pList = mpObject->ForceGluePointList();
        if( pList )
        {
            Index -= 4;
            if( Index >= 0 && Index < pList->GetCount() )
            {
                pList->Delete( (sal_uInt16)Index );

                // only repaint, no objectchange
                mpObject->ActionChanged();
                // mpObject->BroadcastObjectChange();

                return;
            }
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XIndexReplace
void SAL_CALL SvxUnoGluePointAccess::replaceByIndex( sal_Int32 Index, const uno::Any& Element )
    throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException,
    uno::RuntimeException, std::exception)
{
    drawing::GluePoint2 aUnoGlue;
    if(!(Element >>= aUnoGlue))
        throw lang::IllegalArgumentException();

    Index -= 4;
    if( mpObject.is() && Index >= 0 )
    {
        SdrGluePointList* pList = const_cast< SdrGluePointList* >( mpObject->GetGluePointList() );
        if( pList && Index < pList->GetCount() )
        {
            SdrGluePoint& rGlue = (*pList)[(sal_uInt16)Index];
            convert( aUnoGlue, rGlue );

            // only repaint, no objectchange
            mpObject->ActionChanged();
            // mpObject->BroadcastObjectChange();
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XIndexAccess
sal_Int32 SAL_CALL SvxUnoGluePointAccess::getCount()
    throw(uno::RuntimeException, std::exception)
{
    sal_Int32 nCount = 0;
    if( mpObject.is() )
    {
        // each node has a default of 4 glue points
        // and any number of user defined glue points
        if( mpObject->IsNode() )
        {
            nCount += 4;

            const SdrGluePointList* pList = mpObject->GetGluePointList();
            if( pList )
                nCount += pList->GetCount();
        }
    }

    return nCount;
}

uno::Any SAL_CALL SvxUnoGluePointAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( Index >= 0 && mpObject.is() && mpObject->IsNode() )
    {
        struct drawing::GluePoint2 aGluePoint;

        if( Index < 4 ) // default glue point?
        {
            SdrGluePoint aTempPoint = mpObject->GetVertexGluePoint( (sal_uInt16)Index );
            aGluePoint.IsUserDefined = sal_False;
            convert( aTempPoint, aGluePoint );
            return uno::Any(aGluePoint);
        }
        else
        {
            Index -= 4;
            const SdrGluePointList* pList = mpObject->GetGluePointList();
            if( pList && Index < pList->GetCount() )
            {
                const SdrGluePoint& rTempPoint = (*pList)[(sal_uInt16)Index];
                aGluePoint.IsUserDefined = sal_True;
                convert( rTempPoint, aGluePoint );
                return uno::Any(aGluePoint);
            }
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoGluePointAccess::getElementType()
    throw( uno::RuntimeException, std::exception)
{
    return cppu::UnoType<drawing::GluePoint2>::get();
}

sal_Bool SAL_CALL SvxUnoGluePointAccess::hasElements()
    throw( uno::RuntimeException, std::exception)
{
    return mpObject.is() && mpObject->IsNode();
}

/**
 * Create a SvxUnoGluePointAccess
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoGluePointAccess_createInstance( SdrObject* pObject )
{
    return *new SvxUnoGluePointAccess(pObject);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
