/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/container/XIdentifierContainer.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <cppuhelper/implbase2.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdrglue.hxx>
#include <svx/svdpage.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::cppu;

const sal_uInt16 NON_USER_DEFINED_GLUE_POINTS = 4;

class SvxUnoGluePointAccess : public WeakImplHelper2< container::XIndexContainer, container::XIdentifierContainer >
{
private:
    SdrObjectWeakRef    mpObject;

public:
    SvxUnoGluePointAccess( SdrObject* pObject ) throw();
    virtual ~SvxUnoGluePointAccess() throw();

    // XIdentifierContainer
    virtual sal_Int32 SAL_CALL insert( const uno::Any& aElement ) throw (lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XIdentifierReplace
    virtual void SAL_CALL replaceByIdentifer( sal_Int32 Identifier, const uno::Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XIdentifierReplace
    virtual uno::Any SAL_CALL getByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence< sal_Int32 > SAL_CALL getIdentifiers(  ) throw (uno::RuntimeException);

    /* deprecated */
    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    /* deprecated */
    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    /* deprecated */
    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(uno::RuntimeException);
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw( uno::RuntimeException);
};

SvxUnoGluePointAccess::SvxUnoGluePointAccess( SdrObject* pObject ) throw()
: mpObject( pObject )
{
}

SvxUnoGluePointAccess::~SvxUnoGluePointAccess() throw()
{
}

// XIdentifierContainer
sal_Int32 SAL_CALL SvxUnoGluePointAccess::insert( const uno::Any& aElement ) throw (lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpObject.is() )
    {
        sdr::glue::List* pList = mpObject->GetGluePointList(true);

        if( pList )
        {
            drawing::GluePoint2 aUnoGlue;

            if( aElement >>= aUnoGlue )
            {
                const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
                const sdr::glue::Point aSdrGlue(aUnoGlue, aAbsoluteScale);
                const sdr::glue::Point& rAdded = pList->add(aSdrGlue);

                // only repaint, no objectchange
                mpObject->ActionChanged();

                return rAdded.getID() + NON_USER_DEFINED_GLUE_POINTS;
            }

            throw lang::IllegalArgumentException();
        }
    }

    return -1;
}

void SAL_CALL SvxUnoGluePointAccess::removeByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpObject.is() && ( Identifier >= NON_USER_DEFINED_GLUE_POINTS ))
    {
        const sal_uInt32 nId(Identifier - NON_USER_DEFINED_GLUE_POINTS);
        sdr::glue::List* pList = mpObject->GetGluePointList(false);

        if(pList)
        {
            const sdr::glue::Point* pCandidate = pList->findByID(nId);

            if(pCandidate)
            {
                pList->remove(*pCandidate);

                // only repaint, no objectchange
                mpObject->ActionChanged();

                return;
            }
        }

        // TTTT:GLUE
        //const sal_uInt32 nCount = pList ? pList->GetCount() : 0;
        //sal_uInt32 i;
        //
        //for( i = 0; i < nCount; i++ )
        //{
        //    if( (*pList)[i].GetId() == nId )
        //    {
        //        pList->Delete( i );
        //
        //        // only repaint, no objectchange
        //        mpObject->ActionChanged();
        //
        //        return;
        //    }
        //}
    }

    throw container::NoSuchElementException();
}

// XIdentifierReplace
void SAL_CALL SvxUnoGluePointAccess::replaceByIdentifer( sal_Int32 Identifier, const uno::Any& aElement ) throw (lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(mpObject.is())
    {
        struct drawing::GluePoint2 aGluePoint;

        if( (Identifier < NON_USER_DEFINED_GLUE_POINTS) || !(aElement >>= aGluePoint))
            throw lang::IllegalArgumentException();

        const sal_uInt32 nId(Identifier - NON_USER_DEFINED_GLUE_POINTS);
        sdr::glue::List* pList = mpObject->GetGluePointList(false);

        if(pList)
        {
            sdr::glue::Point* pCandidate = pList->findByID(nId);

            if(pCandidate)
            {
                // change the glue point (but start with a new GluePoint)
                const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
                const sdr::glue::Point aSdrGlue(aGluePoint, aAbsoluteScale);

                *pCandidate = aSdrGlue;

                // only repaint, no objectchange
                mpObject->ActionChanged();

                return;
            }
        }

        // TTTT:GLUE
        //const sal_uInt32 nCount = pList ? pList->GetCount() : 0;
        //sal_uInt32 i;
        //for( i = 0; i < nCount; i++ )
        //{
        //    if( (*pList)[i].GetId() == nId )
        //    {
        //        // change the glue point
        //        sdr::glue::Point& rTempPoint = (*pList)[i];
        //        convert( aGluePoint, rTempPoint );
        //
        //        // only repaint, no objectchange
        //        mpObject->ActionChanged();
        //
        //        return;
        //    }
        //}

        throw container::NoSuchElementException();
    }
}

// XIdentifierAccess
uno::Any SAL_CALL SvxUnoGluePointAccess::getByIdentifier( sal_Int32 Identifier ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(mpObject.is())
    {
        if( Identifier < NON_USER_DEFINED_GLUE_POINTS ) // default glue point?
        {
            const sdr::glue::Point aTempPoint(mpObject->GetVertexGluePoint(Identifier));
            const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
            const drawing::GluePoint2 aGluePoint(aTempPoint.convertToGluePoint2(aAbsoluteScale));

            return uno::makeAny( aGluePoint );
        }
        else
        {
            const sal_uInt32 nId(Identifier - NON_USER_DEFINED_GLUE_POINTS);
            const sdr::glue::List* pList = mpObject->GetGluePointList(false);

            if(pList)
            {
                const sdr::glue::Point* pCandidate = pList->findByID(nId);

                if(pCandidate)
                {
                    const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
                    const drawing::GluePoint2 aGluePoint(pCandidate->convertToGluePoint2(aAbsoluteScale));

                    return uno::makeAny( aGluePoint );
                }
            }

            // TTTT:GLUE
            //const sal_uInt32 nCount = pList ? pList->GetCount() : 0;
            //
            //for( sal_uInt32 i = 0; i < nCount; i++ )
            //{
            //    const sdr::glue::Point& rTempPoint = (*pList)[i];
            //    if( rTempPoint.GetId() == nId )
            //    {
            //        // #i38892#
            //        if(rTempPoint.IsUserDefined())
            //        {
            //            aGluePoint.IsUserDefined = sal_True;
            //        }
            //
            //        convert( rTempPoint, aGluePoint );
            //        return uno::makeAny( aGluePoint );
            //    }
            //}
        }
    }

    throw lang::IndexOutOfBoundsException();
}

uno::Sequence< sal_Int32 > SAL_CALL SvxUnoGluePointAccess::getIdentifiers() throw (uno::RuntimeException)
{
    if( mpObject.is() )
    {
        const sdr::glue::List* pList = mpObject->GetGluePointList(false);
        const sdr::glue::PointVector aGluePointVector(pList ? pList->getVector() : sdr::glue::PointVector());
        uno::Sequence< sal_Int32 > aIdSequence(aGluePointVector.size() + NON_USER_DEFINED_GLUE_POINTS);
        sal_Int32 *pIdentifier = aIdSequence.getArray();
        sal_uInt32 i(0);

        for(i = 0; i < NON_USER_DEFINED_GLUE_POINTS; i++)
        {
            *pIdentifier++ = i;
        }

        for(i = 0; i < aGluePointVector.size(); i++)
        {
            const sdr::glue::Point* pCandidate = aGluePointVector[i];

            if(pCandidate)
            {
                *pIdentifier++ = pCandidate->getID() + NON_USER_DEFINED_GLUE_POINTS;
            }
            else
            {
                OSL_ENSURE(false, "Got a sdr::glue::PointVector with empty entries (!)");
            }
        }

        // TTTT:GLUE
        //const sal_uInt32 nCount = pList ? pList->GetCount() : 0;
        //sal_uInt32 i;
        //
        //uno::Sequence< sal_Int32 > aIdSequence( nCount + NON_USER_DEFINED_GLUE_POINTS );
        //sal_Int32 *pIdentifier = aIdSequence.getArray();
        //
        //for( i = 0; i < NON_USER_DEFINED_GLUE_POINTS; i++ )
        //    *pIdentifier++ = i;
        //
        //for( i = 0; i < nCount; i++ )
        //    *pIdentifier++ = ( (*pList)[i].GetId() + NON_USER_DEFINED_GLUE_POINTS ) - 1;

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
void SAL_CALL SvxUnoGluePointAccess::insertByIndex( sal_Int32, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpObject.is() )
    {
        sdr::glue::List* pList = mpObject->GetGluePointList(true);

        if( pList )
        {
            drawing::GluePoint2 aUnoGlue;

            if( Element >>= aUnoGlue )
            {
                const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
                const sdr::glue::Point aSdrGlue(aUnoGlue, aAbsoluteScale);

                pList->add(aSdrGlue);

                // only repaint, no objectchange
                mpObject->ActionChanged();

                return;
            }

            throw lang::IllegalArgumentException();
        }
    }

    throw lang::IndexOutOfBoundsException();
}

void SAL_CALL SvxUnoGluePointAccess::removeByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpObject.is() )
    {
        sdr::glue::List* pList = mpObject->GetGluePointList(true);

        if( pList )
        {
            Index -= 4;

            if(Index >= 0)
            {
                const sdr::glue::Point* pCandidate = pList->findByID(Index);

                if(pCandidate)
                {
                    pList->remove(*pCandidate);

                    // only repaint, no objectchange
                    mpObject->ActionChanged();

                    return;
                }

                // TTTT:GLUE
                //{
                //    const sdr::glue::Point* pCandidate = pList->findByID(sal_uInt32 nID) const;
                //
                //
                //
                //    pList->Delete( (sal_uInt32)Index );
                //
                //    // only repaint, no objectchange
                //    mpObject->ActionChanged();
                //
                //    return;
                //}
            }
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XIndexReplace
void SAL_CALL SvxUnoGluePointAccess::replaceByIndex( sal_Int32 Index, const uno::Any& Element ) throw(lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    drawing::GluePoint2 aUnoGlue;

    if(!(Element >>= aUnoGlue))
        throw lang::IllegalArgumentException();

    Index -= 4;

    if( mpObject.is() && Index >= 0 )
    {
        const sdr::glue::List* pList = mpObject->GetGluePointList(false);
        const sdr::glue::PointVector aGluePointVector(pList ? pList->getVector() : sdr::glue::PointVector());

        if(Index < (sal_Int32)aGluePointVector.size())
        {
            sdr::glue::Point* pCandidate = aGluePointVector[Index];

            if(pCandidate)
            {
                const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
                const sdr::glue::Point aSdrGlue(aUnoGlue, aAbsoluteScale);

                *pCandidate = aSdrGlue;

                // only repaint, no objectchange
                mpObject->ActionChanged();
            }
            else
            {
                OSL_ENSURE(false, "Got a sdr::glue::PointVector with empty entries (!)");
            }

            // TTTT:GLUE
            //rGlue = (*pList)[(sal_uInt32)Index];
            //convert( aUnoGlue, rGlue );
            //
            //// only repaint, no objectchange
            //mpObject->ActionChanged();
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XIndexAccess
sal_Int32 SAL_CALL SvxUnoGluePointAccess::getCount() throw(uno::RuntimeException)
{
    sal_Int32 nCount = 0;

    if( mpObject.is() )
    {
        // each node has a default of 4 glue points
        // and any number of user defined glue points
        nCount += 4;

        const sdr::glue::List* pList = mpObject->GetGluePointList(false);
        const sdr::glue::PointVector aGluePointVector(pList ? pList->getVector() : sdr::glue::PointVector());

        nCount += aGluePointVector.size();
    }

    return nCount;
}

uno::Any SAL_CALL SvxUnoGluePointAccess::getByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if(Index >= 0 && mpObject.is())
    {
        if( Index < 4 ) // default glue point?
        {
            const sdr::glue::Point aTempPoint(mpObject->GetVertexGluePoint(Index));
            const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
            const drawing::GluePoint2 aGluePoint(aTempPoint.convertToGluePoint2(aAbsoluteScale));
            uno::Any aAny;

            aAny <<= aGluePoint;
            return aAny;
        }
        else
        {
            Index -= 4;
            const sdr::glue::List* pList = mpObject->GetGluePointList(false);
            const sdr::glue::PointVector aGluePointVector(pList ? pList->getVector() : sdr::glue::PointVector());

            if(pList && Index < (sal_Int32)aGluePointVector.size())
            {
                const sdr::glue::Point* pCandidate = aGluePointVector[Index];

                if(pCandidate)
                {
                    const basegfx::B2DVector aAbsoluteScale(basegfx::absolute(mpObject->getSdrObjectScale()));
                    const drawing::GluePoint2 aGluePoint(pCandidate->convertToGluePoint2(aAbsoluteScale));
                    uno::Any aAny;

                    aAny <<= aGluePoint;
                    return aAny;
                }
                else
                {
                    OSL_ENSURE(false, "Got a sdr::glue::PointVector with empty entries (!)");
                }

                // TTTT:GLUE
                //rTempPoint = (*pList)[(sal_uInt32)Index];
                //aGluePoint.IsUserDefined = sal_True;
                //convert( rTempPoint, aGluePoint );
                //uno::Any aAny;
                //aAny <<= aGluePoint;
                //return aAny;
            }
        }
    }

    throw lang::IndexOutOfBoundsException();
}

// XElementAccess
uno::Type SAL_CALL SvxUnoGluePointAccess::getElementType() throw( uno::RuntimeException)
{
    return ::getCppuType((const struct drawing::GluePoint2*)0);
}

sal_Bool SAL_CALL SvxUnoGluePointAccess::hasElements() throw( uno::RuntimeException)
{
    return mpObject.is();
}

/**
 * Create a SvxUnoGluePointAccess
 */
uno::Reference< uno::XInterface > SAL_CALL SvxUnoGluePointAccess_createInstance( SdrObject* pObject )
{
    return *new SvxUnoGluePointAccess(pObject);
}

// eof
