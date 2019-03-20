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
                bLoop = true;
            }

            // to see the diff between IAdministrator_vector and
            // IAdministrator_set, see instructions in CntMultiValue above,
            // create version for IAdministrator_set and raise this number.
            // this demonstrates how/why this should be much faster than
            // current SfxPoolItem/SfxItemSet/SfxItemPool stuff ...
            int nIncrement(0);

            CntMultiValue::SharedPtr mhas3(CntMultiValue::Create(5,2,8));
            CntMultiValue::SharedPtr mhas4(CntMultiValue::Create(2,3,0));

            if(CntMultiValue::IsDefault(CntMultiValue::Create(3,0,0)))
            {
                nIncrement++;
            }

            if(CntMultiValue::IsDefault(CntMultiValue::Create(8,7,6)))
            {
                nIncrement++;
            }

            if(CntMultiValue::IsDefault(CntMultiValue::GetDefault()))
            {
                nIncrement++;
            }

            CntMultiValue::SharedPtr testAA(CntMultiValue::Create(11,22,99));
            std::vector<CntMultiValue::SharedPtr> test;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                test.push_back(CntMultiValue::Create(aloop+1, (aloop+1)*2, (aloop+1)*4));
            }

            CntMultiValue::SharedPtr testA(CntMultiValue::Create(2,4,8));
            CntMultiValue::SharedPtr testB(CntMultiValue::Create(2,4,9));

            for(sal_uInt32 dloop(0); dloop < nLoopNumber; dloop+=2)
            {
                test[dloop] = nullptr;
                if(dloop%5)
                    test.push_back(CntMultiValue::Create(dloop+1, (dloop+1)*2, (dloop+1)*4));
            }

            for(sal_uInt32 eloop(1); eloop < (nLoopNumber * 2) / 3; eloop+=2)
            {
                test[eloop] = nullptr;
                if(eloop%7)
                    test.push_back(CntMultiValue::Create(eloop+1, (eloop+1)*2, (eloop+1)*4));
            }

            test.clear();
        }

        void checkCntMultiValueAtISet()
        {
            int nIncrement(0);

            // make use of local CntMultiValue item in conjuction with ISet
            ModelSpecificIValues::SharedPtr aModelSpecificIValues(ModelSpecificIValues::Create());
            ISet::SharedPtr aSet(ISet::Create(aModelSpecificIValues));
            aSet->SetItem(CntMultiValue::Create(5,4,3));

            if(const auto Item(aSet->GetStateAndItem<const CntMultiValue>()); Item.IsSet())
            {
                nIncrement += (ISet::IState::SET == Item.GetIState()) ? 1 : 0;
                nIncrement += (Item.GetItem()) ? 1 : 0;
                nIncrement += (nullptr != Item.GetItemInstance()) ? 1 : 0;
                nIncrement += Item.HasItem();
                nIncrement += Item.IsDisabled();
            }
            else if(Item.IsDefault())
            {
                nIncrement += Item.IsDisabled();
            }
            else if(Item.IsDontCare())
            {
                nIncrement += Item.IsDisabled();
            }
            else
            {
                nIncrement += Item.IsDisabled();
            }

            // CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
            //                              B2IRange(1, 2, 4, 5), fround(ibase(1.2, 2.3, 3.5, 4.8)));
        }

        void checkSimpleItems()
        {
            static bool bInit(false);
            static CntInt16::SharedPtr a_sp, b_sp, c_sp;
            static CntInt16::SharedPtr theDefault;
            static CntOUString::SharedPtr sa_sp, sb_sp, sc_sp;
            static CntOUString::SharedPtr stheDefault;
            const sal_uInt32 nLoopNumber(50);
            int nIncrement(0);

            if(!bInit)
            {
                bInit = true;
                a_sp = CntInt16::Create(3);
                b_sp = CntInt16::Create(5);
                c_sp = CntInt16::Create(7);
                theDefault = CntInt16::GetDefault();

                sa_sp = CntOUString::Create("Hello");
                sb_sp = CntOUString::Create("World");
                sc_sp = CntOUString::Create("..of Doom!");
                stheDefault = CntOUString::GetDefault();
            }

            ///////////////////////////////////////////////////////////////////////////////

            CntInt16::SharedPtr has3(CntInt16::Create(3));
            CntInt16::SharedPtr has4(CntInt16::Create(4));

            if(CntInt16::IsDefault(CntInt16::Create(11)))
            {
                nIncrement++;
            }

            if(CntInt16::IsDefault(CntInt16::Create(0)))
            {
                nIncrement++;
            }

            if(CntInt16::IsDefault(theDefault))
            {
                nIncrement++;
            }

            std::vector<CntInt16::SharedPtr> test16;

            for(sal_uInt32 a(0); a < nLoopNumber; a++)
            {
                test16.push_back(CntInt16::Create(a));
            }

            test16.clear();

            ///////////////////////////////////////////////////////////////////////////////

            CntOUString::SharedPtr shas3(CntOUString::Create("Hello"));
            CntOUString::SharedPtr shas4(CntOUString::Create("WhateverComesAlong"));

            if(CntOUString::IsDefault(CntOUString::Create("NotDefault")))
            {
                nIncrement++;
            }

            if(CntOUString::IsDefault(CntOUString::Create(OUString())))
            {
                nIncrement++;
            }

            if(CntOUString::IsDefault(stheDefault))
            {
                nIncrement++;
            }

            std::vector<CntOUString::SharedPtr> testStr;

            for(sal_uInt32 a(0); a < nLoopNumber; a++)
            {
                testStr.push_back(CntOUString::Create(OUString::number(static_cast<int>(a))));
            }

            testStr.clear();

            if(bInit)
            {
                bInit = false;
                a_sp.reset();
                b_sp.reset();
                c_sp.reset();
                theDefault.reset();
                sa_sp.reset();
                sb_sp.reset();
                sc_sp.reset();
                stheDefault.reset();
            }
        }

        void checkSimpleItemsAtISet()
        {
            int nIncrement(0);

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
                nIncrement += (ISet::IState::SET == Item.GetIState()) ? 1 : 0;
                nIncrement += (Item.GetItem()) ? 1 : 0;
                nIncrement += (nullptr != Item.GetItemInstance()) ? 1 : 0;
                nIncrement += Item.HasItem();
                nIncrement += Item.IsDisabled();
            }
            else if(Item.IsDefault())
            {
                nIncrement += Item.IsDisabled();
            }
            else if(Item.IsDontCare())
            {
                nIncrement += Item.IsDisabled();
            }
            else
            {
                nIncrement += Item.IsDisabled();
            }

            // check getting default at Set, this will include ModelSpecificIValues
            // compared with the static CntInt16::GetDefault() call
            const CntInt16::SharedPtr aDefSet(aSet->GetDefault<const CntInt16>());
            const CntInt16::SharedPtr aDefGlobal(CntInt16::GetDefault());

            const bool bClA(aSet->ClearItem<const CntInt16>());
            nIncrement += bClA;
            // let one exist to check destruction when Set gets destructed
            // const bool bClB(aSet->ClearItem<const CntOUString>());
            nIncrement ++;
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
