/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <item/base/IBase.hxx>
#include <item/base/IAdministrator.hxx>
#include <item/base/ModelSpecificIValues.hxx>
#include <item/base/ISet.hxx>
#include <item/simple/CntInt16.hxx>
#include <item/simple/CntOUString.hxx>

namespace Item
{
    // example for multi value Item
    class CntMultiValue;
    typedef IBaseStaticHelper<CntMultiValue, IAdministrator_vector> CntMultiValueStaticHelper;

    // if this should be based on faster IAdministrator_set, changes needed would be:
    // - typedef IBaseStaticHelper<CntMultiValue, IAdministrator_set> CntMultiValueStaticHelper;
    // - define virtual bool operator<(const IBase& rCandidate) const override

    class CntMultiValue : public CntMultiValueStaticHelper
    {
    private:
        sal_Int16 m_nValueA;
        sal_Int32 m_nValueB;
        sal_Int64 m_nValueC;

    protected:
        friend CntMultiValueStaticHelper;

        CntMultiValue(sal_Int16 nValueA = 0, sal_Int16 nValueB = 0, sal_Int16 nValueC = 0)
        :   CntMultiValueStaticHelper(),
            m_nValueA(nValueA),
            m_nValueB(nValueB),
            m_nValueC(nValueC)
        {
        }

    public:
        virtual ~CntMultiValue()
        {
            // needs to be called from here to have the fully derived implementation type
            // in the helper method - do NOT move to a imaginable general
            // implementation in IBaseStaticHelper (!)
            if(IsAdministrated())
            {
                GetStaticAdmin().HintExpired(this);
            }
        }

        static std::shared_ptr<const CntMultiValue> Create(sal_Int16 nValueA, sal_Int16 nValueB, sal_Int16 nValueC)
        {
            // use ::Create(...) method with local incarnation, it will handle
            // - detection of being default (will delete local incarnation)
            // - detection of reuse (will delete local incarnation)
            // - detectiomn of new use - will create shared_ptr for local incarnation and buffer
            return std::static_pointer_cast<const CntMultiValue>(GetStaticAdmin().Create(new CntMultiValue(nValueA, nValueB, nValueC)));
        }

        virtual bool operator==(const IBase& rCandidate) const override
        {
            assert(IBase::operator==(rCandidate));
            const CntMultiValue& rCand(static_cast<const CntMultiValue&>(rCandidate));
            return (GetValueA() == rCand.GetValueA()
                && GetValueB() == rCand.GetValueB()
                && GetValueC() == rCand.GetValueC());
        }

        sal_Int16 GetValueA() const
        {
            return m_nValueA;
        }

        sal_Int16 GetValueB() const
        {
            return m_nValueB;
        }

        sal_Int16 GetValueC() const
        {
            return m_nValueC;
        }

        // virtual bool operator<(const IBase& rCandidate) const override
        // {
        //     assert(IBase::operator==(rCandidate));
        //     return static_cast<const CntMultiValue*>(this)->GetValueA() < static_cast<const CntMultiValue&>(rCandidate).GetValueA()
        //         && static_cast<const CntMultiValue*>(this)->GetValueB() < static_cast<const CntMultiValue&>(rCandidate).GetValueB()
        //         && static_cast<const CntMultiValue*>(this)->GetValueC() < static_cast<const CntMultiValue&>(rCandidate).GetValueC();
        // }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ibase : public CppUnit::TestFixture
    {
    private:
    public:
        void checkCntMultiValue()
        {
            // make direct use of local CntMultiValue item

            // for debug, change bLoop to true, start, attach and set to
            // false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bool bHhklsefh = true;
            }

            // to see the diff between IAdministrator_vector and
            // IAdministrator_set, see instructions in CntMultiValue above,
            // create version for IAdministrator_set and raise this number.
            // this demonstrates how/why this should be much faster than
            // current SfxPoolItem/SfxItemSet/SfxItemPool stuff ...
            static sal_uInt32 nLoopNumber(50);

            CntMultiValue::SharedPtr mhas3(CntMultiValue::Create(5,2,8));
            CntMultiValue::SharedPtr mhas4(CntMultiValue::Create(2,3,0));

            if(CntMultiValue::IsDefault(CntMultiValue::Create(3,0,0)))
            {
                bool bBla8 = true;
            }

            if(CntMultiValue::IsDefault(CntMultiValue::Create(8,7,6)))
            {
                bool bBla9 = true;
            }

            if(CntMultiValue::IsDefault(CntMultiValue::GetDefault()))
            {
                bool bBla10 = true;
            }

            if(true)
            {
                CntMultiValue::SharedPtr testAA(CntMultiValue::Create(11,22,99));
                std::vector<CntMultiValue::SharedPtr> test;
                for(sal_uInt32 a(0); a < nLoopNumber; a++)
                {
                    test.push_back(CntMultiValue::Create(a+1, (a+1)*2, (a+1)*4));
                }
                CntMultiValue::SharedPtr testA(CntMultiValue::Create(2,4,8));
                CntMultiValue::SharedPtr testB(CntMultiValue::Create(2,4,9));
                for(sal_uInt32 d(0); d < nLoopNumber; d+=2)
                {
                    test[d] = nullptr;
                    if(d%5)
                        test.push_back(CntMultiValue::Create(d+1, (d+1)*2, (d+1)*4));
                }
                for(sal_uInt32 e(1); e < (nLoopNumber * 2) / 3; e+=2)
                {
                    test[e] = nullptr;
                    if(e%7)
                        test.push_back(CntMultiValue::Create(e+1, (e+1)*2, (e+1)*4));
                }
                test.clear();
            }
        }

        void checkCntMultiValueAtISet()
        {
            // make use of local CntMultiValue item in conjuction with ISet
            ModelSpecificIValues::SharedPtr aModelSpecificIValues(ModelSpecificIValues::Create());
            ISet::SharedPtr aSet(ISet::Create(aModelSpecificIValues));
            aSet->SetItem(CntMultiValue::Create(5,4,3));

            if(const auto Item(aSet->GetStateAndItem<const CntMultiValue>()); Item.IsSet())
            {
                const ISet::IState aSAAState = Item.GetIState();
                const CntMultiValue::SharedPtr aSAAItem = Item.GetItem();
                const CntMultiValue* pSAAPtr = Item.GetItemInstance();
                bool bSAAa = Item.HasItem();
                bool bSAAe = Item.IsDisabled();
            }
            else if(Item.IsDefault())
            {
                bool bSAAe = Item.IsDisabled();
            }
            else if(Item.IsDontCare())
            {
                bool bSAAe = Item.IsDisabled();
            }
            else
            {
                bool bSAAe = Item.IsDisabled();
            }

            // CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
            //                              B2IRange(1, 2, 4, 5), fround(ibase(1.2, 2.3, 3.5, 4.8)));
        }

        void checkSimpleItems()
        {
            static bool bInit(false);
            static CntInt16::SharedPtr a, b, c;
            static CntInt16::SharedPtr theDefault;
            static CntOUString::SharedPtr sa, sb, sc;
            static CntOUString::SharedPtr stheDefault;
            static sal_uInt32 nLoopNumber(50);

            if(!bInit)
            {
                bInit = true;
                a = CntInt16::Create(3);
                b = CntInt16::Create(5);
                c = CntInt16::Create(7);
                theDefault = CntInt16::GetDefault();

                sa = CntOUString::Create("Hello");
                sb = CntOUString::Create("World");
                sc = CntOUString::Create("..of Doom!");
                stheDefault = CntOUString::GetDefault();
            }

            ///////////////////////////////////////////////////////////////////////////////

            CntInt16::SharedPtr has3(CntInt16::Create(3));
            CntInt16::SharedPtr has4(CntInt16::Create(4));

            if(CntInt16::IsDefault(CntInt16::Create(11)))
            {
                bool bBla2 = true;
            }

            if(CntInt16::IsDefault(CntInt16::Create(0)))
            {
                bool bBla3 = true;
            }

            if(CntInt16::IsDefault(theDefault))
            {
                bool bBla4 = true;
            }

            if(true)
            {
                std::vector<CntInt16::SharedPtr> test16;
                for(sal_uInt32 a(0); a < nLoopNumber; a++)
                {
                    test16.push_back(CntInt16::Create(a));
                }
                test16.clear();
            }

            ///////////////////////////////////////////////////////////////////////////////

            CntOUString::SharedPtr shas3(CntOUString::Create("Hello"));
            CntOUString::SharedPtr shas4(CntOUString::Create("WhateverComesAlong"));

            if(CntOUString::IsDefault(CntOUString::Create("NotDefault")))
            {
                bool bBla5 = true;
            }

            if(CntOUString::IsDefault(CntOUString::Create(OUString())))
            {
                bool bBla6 = true;
            }

            if(CntOUString::IsDefault(stheDefault))
            {
                bool bBla7 = true;
            }

            if(true)
            {
                std::vector<CntOUString::SharedPtr> testStr;
                for(sal_uInt32 a(0); a < nLoopNumber; a++)
                {
                    testStr.push_back(CntOUString::Create(OUString::number(static_cast<int>(a))));
                }
                testStr.clear();
            }

            if(bInit)
            {
                bInit = false;
                a.reset();
                b.reset();
                c.reset();
                theDefault.reset();
                sa.reset();
                sb.reset();
                sc.reset();
                stheDefault.reset();
            }
        }

        void checkSimpleItemsAtISet()
        {
            ModelSpecificIValues::SharedPtr aModelSpecificIValues(ModelSpecificIValues::Create());
            aModelSpecificIValues->SetAlternativeDefaultItem(CntInt16::Create(3));
            aModelSpecificIValues->SetAlternativeDefaultItem(CntInt16::Create(4));

            ISet::SharedPtr aSet(ISet::Create(aModelSpecificIValues));
            const auto aActEmpty(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntInt16::Create(4));
            const auto aActA(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntInt16::GetDefault());
            const auto aActB(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntInt16::Create(12));
            const auto aActC(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntOUString::Create("Teststring - not really useful :-)"));
            const auto aActStr(aSet->GetStateAndItem<const CntOUString>());

            const auto ItemDADA(aSet->GetStateAndItem<const CntInt16>());

            if(const auto Item(aSet->GetStateAndItem<const CntOUString>()); Item.IsSet())
            {
                const ISet::IState aSAAState = Item.GetIState();
                const CntOUString::SharedPtr aSAAItem = Item.GetItem();
                const CntOUString* pSAAPtr = Item.GetItemInstance();
                bool bSAAa = Item.HasItem();
                bool bSAAe = Item.IsDisabled();
            }
            else if(Item.IsDefault())
            {
                bool bSAAe = Item.IsDisabled();
            }
            else if(Item.IsDontCare())
            {
                bool bSAAe = Item.IsDisabled();
            }
            else
            {
                bool bSAAe = Item.IsDisabled();
            }

            // check getting default at Set, this will include ModelSpecificIValues
            // compared with the static CntInt16::GetDefault() call
            const CntInt16::SharedPtr aDefSet(aSet->GetDefault<const CntInt16>());
            const CntInt16::SharedPtr aDefGlobal(CntInt16::GetDefault());

            const bool bClA(aSet->ClearItem<const CntInt16>());
            // let one exist to check destruction when Set gets destructed
            // const bool bClB(aSet->ClearItem<const CntOUString>());
            bool bBla23 = true;
        }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ibase);
        CPPUNIT_TEST(checkCntMultiValue);
        CPPUNIT_TEST(checkCntMultiValueAtISet);
        CPPUNIT_TEST(checkSimpleItems);
        CPPUNIT_TEST(checkSimpleItemsAtISet);
        CPPUNIT_TEST_SUITE_END();
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(Item::ibase);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
