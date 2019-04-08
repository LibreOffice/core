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

#include <item/base/ItemBase.hxx>
#include <item/base/ItemAdministrator.hxx>
#include <item/base/ModelSpecificItemValues.hxx>
#include <item/base/ItemSet.hxx>
#include <item/simple/CntInt16.hxx>
#include <item/simple/CntOUString.hxx>

namespace Item
{
    // example for multi value Item
    class MultiValueAB;
    typedef ItemBaseStaticHelper<MultiValueAB, IAdministrator_vector> MultiValueStaticHelperAB;

    // if this should be based on faster IAdministrator_set, changes needed would be:
    // - typedef ItemBaseStaticHelper<MultiValueAB, IAdministrator_set> MultiValueStaticHelperAB;
    // - define virtual bool operator<(const ItemBase& rCandidate) const override

    class MultiValueAB : public MultiValueStaticHelperAB, public ItemBase
    {
    public:
        // SharedPtr typedef to be used handling instances of given type
        typedef std::shared_ptr<const MultiValueAB> SharedPtr;

    private:
        // need to offer internal access to ItemAdministrator
        // in each derivation - just calls GetStaticAdmin internally
        virtual ItemAdministrator* GetIAdministrator() const override
        {
            return &MultiValueStaticHelperAB::GetStaticAdmin();
        }

    private:
        sal_Int16 m_nValueA;
        sal_Int32 m_nValueB;

    protected:
        friend MultiValueStaticHelperAB;

        MultiValueAB(sal_Int16 nValueA = 0, sal_Int16 nValueB = 0)
        :   MultiValueStaticHelperAB(),
            ItemBase(),
            m_nValueA(nValueA),
            m_nValueB(nValueB)
        {
        }

    public:
        virtual ~MultiValueAB()
        {
            // needs to be called from here to have the fully derived implementation type
            // in the helper method - do NOT move to a imaginable general
            // implementation in ItemBaseStaticHelper (!)
            implInstanceCleanup();
        }

        static std::shared_ptr<const MultiValueAB> Create(sal_Int16 nValueA, sal_Int16 nValueB)
        {
            // use ::Create(...) method with local incarnation, it will handle
            // - detection of being default (will delete local incarnation)
            // - detection of reuse (will delete local incarnation)
            // - detectiomn of new use - will create shared_ptr for local incarnation and buffer
            return std::static_pointer_cast<const MultiValueAB>(
                MultiValueStaticHelperAB::GetStaticAdmin().Create(
                    new MultiValueAB(nValueA, nValueB)));
        }

        virtual bool operator==(const ItemBase& rCandidate) const override
        {
            assert(ItemBase::operator==(rCandidate));
            const MultiValueAB& rCand(static_cast<const MultiValueAB&>(rCandidate));
            return (GetValueA() == rCand.GetValueA()
                && GetValueB() == rCand.GetValueB());
        }

        sal_Int16 GetValueA() const
        {
            return m_nValueA;
        }

        sal_Int16 GetValueB() const
        {
            return m_nValueB;
        }

        // virtual bool operator<(const ItemBase& rCandidate) const override
        // {
        //     assert(ItemBase::operator==(rCandidate));
        //     return static_cast<const MultiValueAB*>(this)->GetValueA() < static_cast<const MultiValueAB&>(rCandidate).GetValueA()
        //         && static_cast<const MultiValueAB*>(this)->GetValueB() < static_cast<const MultiValueAB&>(rCandidate).GetValueB();
        // }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for multi value Item deived from already defined one,
    // adding parameters (only one here)
    class MultiValueABC;
    typedef ItemBaseStaticHelper<MultiValueABC, IAdministrator_vector> MultiValueStaticHelperABC;

    class MultiValueABC : public MultiValueAB, public MultiValueStaticHelperABC
    {
    public:
        // SharedPtr typedef to be used handling instances of given type
        typedef std::shared_ptr<const MultiValueABC> SharedPtr;

    private:
        // need to offer internal access to ItemAdministrator
        // in each derivation - just calls GetStaticAdmin internally
        virtual ItemAdministrator* GetIAdministrator() const override
        {
            return &MultiValueStaticHelperABC::GetStaticAdmin();
        }

    private:
        sal_Int64 m_nValueC;

    protected:
        friend MultiValueStaticHelperABC;

        MultiValueABC(sal_Int16 nValueA = 0, sal_Int16 nValueB = 0, sal_Int16 nValueC = 0)
        :   MultiValueAB(nValueA, nValueB),
            MultiValueStaticHelperABC(),
            m_nValueC(nValueC)
        {
        }

    public:
        virtual ~MultiValueABC()
        {
            // needs to be called from here to have the fully derived implementation type
            // in the helper method - do NOT move to a imaginable general
            // implementation in ItemBaseStaticHelper (!)
            implInstanceCleanup();
        }

        static std::shared_ptr<const MultiValueABC> Create(sal_Int16 nValueA, sal_Int16 nValueB, sal_Int16 nValueC)
        {
            // use ::Create(...) method with local incarnation, it will handle
            // - detection of being default (will delete local incarnation)
            // - detection of reuse (will delete local incarnation)
            // - detectiomn of new use - will create shared_ptr for local incarnation and buffer
            return std::static_pointer_cast<const MultiValueABC>(
                MultiValueStaticHelperABC::GetStaticAdmin().Create(
                    new MultiValueABC(nValueA, nValueB, nValueC)));
        }

        virtual bool operator==(const ItemBase& rCandidate) const override
        {
            assert(ItemBase::operator==(rCandidate));
            const MultiValueABC& rCand(static_cast<const MultiValueABC&>(rCandidate));
            return (MultiValueAB::operator==(rCandidate)
                && GetValueC() == rCand.GetValueC());
        }

        sal_Int16 GetValueC() const
        {
            return m_nValueC;
        }

        // virtual bool operator<(const ItemBase& rCandidate) const override
        // {
        //     assert(ItemBase::operator==(rCandidate));
        //     return MultiValueAB::operator<(rCandidate)
        //         && static_cast<const MultiValueABC*>(this)->GetValueC() < static_cast<const MultiValueABC&>(rCandidate).GetValueC();
        // }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for Item deived from existing ojne, only new type
    class MultiValueAB_Alternative : public MultiValueAB
    {
    public:
        // SharedPtr typedef to be used handling instances of given type
        typedef std::shared_ptr<const MultiValueAB_Alternative> SharedPtr;

    protected:
        MultiValueAB_Alternative(sal_Int16 nValueA = 0, sal_Int16 nValueB = 0)
        :   MultiValueAB(nValueA, nValueB)
        {
        }

    public:
        static std::shared_ptr<const MultiValueAB_Alternative> Create(sal_Int16 nValueA, sal_Int16 nValueB)
        {
            // use ::Create(...) method with local incarnation, it will handle
            // - detection of being default (will delete local incarnation)
            // - detection of reuse (will delete local incarnation)
            // - detectiomn of new use - will create shared_ptr for local incarnation and buffer
            return std::static_pointer_cast<const MultiValueAB_Alternative>(
                MultiValueStaticHelperAB::GetStaticAdmin().Create(
                    new MultiValueAB_Alternative(nValueA, nValueB)));
        }

        virtual bool operator==(const ItemBase& rCandidate) const override
        {
            assert(ItemBase::operator==(rCandidate));
            return (MultiValueAB::operator==(rCandidate));
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ibase : public CppUnit::TestFixture
    {
    private:
    public:
        void checkMultiValue()
        {
            // make direct use of local MultiValueAB item

            // for debug, change bLoop to true, start, attach and set to
            // false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            // to see the diff between IAdministrator_vector and
            // IAdministrator_set, see instructions in MultiValueAB above,
            // create version for IAdministrator_set and raise this number.
            // this demonstrates how/why this should be much faster than
            // current SfxPoolItem/SfxItemSet/SfxItemPool stuff ...
            int nIncrement(0);

            MultiValueAB::SharedPtr mhas3(MultiValueAB::Create(5,2));
            MultiValueAB::SharedPtr mhas4(MultiValueAB::Create(2,3));

            if(MultiValueStaticHelperAB::IsStaticDefault(MultiValueAB::Create(3,0)))
            {
                nIncrement++;
            }

            if(MultiValueStaticHelperAB::IsStaticDefault(MultiValueAB::Create(8,7)))
            {
                nIncrement++;
            }

            if(MultiValueStaticHelperAB::IsStaticDefault(MultiValueStaticHelperAB::GetStaticDefault()))
            {
                nIncrement++;
            }

            if(MultiValueStaticHelperABC::IsStaticDefault(MultiValueStaticHelperABC::GetStaticDefault()))
            {
                nIncrement++;
            }

            MultiValueAB::SharedPtr testAA(MultiValueAB::Create(11,22));
            std::vector<MultiValueAB::SharedPtr> test;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                test.push_back(MultiValueAB::Create(aloop+1, (aloop+1)*2));
            }

            MultiValueAB::SharedPtr testA(MultiValueAB::Create(2,4));
            MultiValueAB::SharedPtr testB(MultiValueAB::Create(2,3));

            for(sal_uInt32 dloop(0); dloop < nLoopNumber; dloop+=2)
            {
                test[dloop] = nullptr;
                if(dloop%5)
                {
                    test.push_back(MultiValueAB::Create(dloop+1, (dloop+1)*2));
                }
            }

            for(sal_uInt32 eloop(1); eloop < (nLoopNumber * 2) / 3; eloop+=2)
            {
                test[eloop] = nullptr;
                if(eloop%7)
                {
                    test.push_back(MultiValueAB::Create(eloop+1, (eloop+1)*2));
                }
            }

            test.clear();
        }

        void checkMultiValueDerived()
        {
            // make direct use of local MultiValueABC item
            int nIncrement(0);

            MultiValueABC::SharedPtr mhas3x(MultiValueABC::Create(5,2,5));
            MultiValueABC::SharedPtr mhas4x(MultiValueABC::Create(2,3,7));

            if(MultiValueStaticHelperABC::IsStaticDefault(MultiValueABC::Create(3,0,5)))
            {
                nIncrement++;
            }

            if(MultiValueStaticHelperABC::IsStaticDefault(MultiValueABC::Create(8,7,12)))
            {
                nIncrement++;
            }

            if(MultiValueStaticHelperABC::IsStaticDefault(MultiValueStaticHelperABC::GetStaticDefault()))
            {
                nIncrement++;
            }

            std::vector<MultiValueABC::SharedPtr> testx;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testx.push_back(MultiValueABC::Create(aloop+1, (aloop+1)*2, (aloop+1)*4));
            }

            MultiValueABC::SharedPtr testAx(MultiValueABC::Create(2,4,3));
            MultiValueABC::SharedPtr testBx(MultiValueABC::Create(2,4,6));

            for(sal_uInt32 dloop(0); dloop < nLoopNumber; dloop+=2)
            {
                testx[dloop] = nullptr;
                if(dloop%5)
                {
                    testx.push_back(MultiValueABC::Create(dloop+1, (dloop+1)*2, (dloop+1)*4));
                }
            }

            for(sal_uInt32 eloop(1); eloop < (nLoopNumber * 2) / 3; eloop+=2)
            {
                testx[eloop] = nullptr;
                if(eloop%7)
                {
                    testx.push_back(MultiValueABC::Create(eloop+1, (eloop+1)*2, 123));
                }
            }

            testx.clear();
        }

        void checkMultiValueAlternative()
        {
            int nIncrement(0);
            const sal_uInt32 nLoopNumber(50);
            MultiValueAB_Alternative::SharedPtr mhas3_A(MultiValueAB_Alternative::Create(5,2));

            if(MultiValueStaticHelperAB::IsStaticDefault(MultiValueAB_Alternative::Create(3,0)))
            {
                nIncrement++;
            }

            if(MultiValueStaticHelperAB::IsStaticDefault(MultiValueAB_Alternative::Create(8,7)))
            {
                nIncrement++;
            }

            if(MultiValueStaticHelperAB::IsStaticDefault(MultiValueStaticHelperAB::GetStaticDefault()))
            {
                nIncrement++;
            }

            std::vector<MultiValueAB_Alternative::SharedPtr> testAB_A;

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testAB_A.push_back(MultiValueAB_Alternative::Create(aloop+1, (aloop+1)*2));
            }

            MultiValueAB_Alternative::SharedPtr testA_A(MultiValueAB_Alternative::Create(2,4));
            MultiValueAB_Alternative::SharedPtr testB_A(MultiValueAB_Alternative::Create(2,3));

            for(sal_uInt32 dloop(0); dloop < nLoopNumber; dloop+=2)
            {
                testAB_A[dloop] = nullptr;
                if(dloop%5)
                {
                    testAB_A.push_back(MultiValueAB_Alternative::Create(dloop+1, (dloop+1)*2));
                }
            }

            for(sal_uInt32 eloop(1); eloop < (nLoopNumber * 2) / 3; eloop+=2)
            {
                testAB_A[eloop] = nullptr;
                if(eloop%7)
                {
                    testAB_A.push_back(MultiValueAB_Alternative::Create(eloop+1, (eloop+1)*2));
                }
            }

            testAB_A.clear();
        }

        void checkMultiValueAtISet()
        {
            int nIncrement(0);

            // make use of local MultiValueAB item in conjuction with ItemSet
            ModelSpecificItemValues::SharedPtr aModelSpecificIValues(ModelSpecificItemValues::Create());
            ItemSet::SharedPtr aSet(ItemSet::Create(aModelSpecificIValues));
            aSet->SetItem(MultiValueAB::Create(5,4));

            if(const auto Item(aSet->GetStateAndItem<const MultiValueAB>()); Item.IsSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.GetIState()) ? 1 : 0;
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
                theDefault = CntInt16::GetStaticDefault();

                sa_sp = CntOUString::Create("Hello");
                sb_sp = CntOUString::Create("World");
                sc_sp = CntOUString::Create("..of Doom!");
                stheDefault = CntOUString::GetStaticDefault();
            }

            ///////////////////////////////////////////////////////////////////////////////

            CntInt16::SharedPtr has3(CntInt16::Create(3));
            CntInt16::SharedPtr has4(CntInt16::Create(4));

            if(CntInt16::IsStaticDefault(CntInt16::Create(11)))
            {
                nIncrement++;
            }

            if(CntInt16::IsStaticDefault(CntInt16::Create(0)))
            {
                nIncrement++;
            }

            if(CntInt16::IsStaticDefault(theDefault))
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

            if(CntOUString::IsStaticDefault(CntOUString::Create("NotDefault")))
            {
                nIncrement++;
            }

            if(CntOUString::IsStaticDefault(CntOUString::Create(OUString())))
            {
                nIncrement++;
            }

            if(CntOUString::IsStaticDefault(stheDefault))
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

            ModelSpecificItemValues::SharedPtr aModelSpecificIValues(ModelSpecificItemValues::Create());
            aModelSpecificIValues->SetAlternativeDefaultItem(CntInt16::Create(3));
            aModelSpecificIValues->SetAlternativeDefaultItem(CntInt16::Create(4));

            ItemSet::SharedPtr aSet(ItemSet::Create(aModelSpecificIValues));
            const auto aActEmpty(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntInt16::Create(4));
            const auto aActA(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntInt16::GetStaticDefault());
            const auto aActB(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntInt16::Create(12));
            const auto aActC(aSet->GetStateAndItem<const CntInt16>());

            aSet->SetItem(CntOUString::Create("Teststring - not really useful :-)"));
            const auto aActStr(aSet->GetStateAndItem<const CntOUString>());

            const auto ItemDADA(aSet->GetStateAndItem<const CntInt16>());

            if(const auto Item(aSet->GetStateAndItem<const CntOUString>()); Item.IsSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.GetIState()) ? 1 : 0;
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

            // check getting default at Set, this will include ModelSpecificItemValues
            // compared with the static CntInt16::GetStaticDefault() call
            const CntInt16::SharedPtr aDefSet(aSet->GetDefault<const CntInt16>());
            const CntInt16::SharedPtr aDefGlobal(CntInt16::GetStaticDefault());

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
        CPPUNIT_TEST(checkMultiValue);
        CPPUNIT_TEST(checkMultiValueDerived);
//        CPPUNIT_TEST(checkMultiValueAlternative);
        CPPUNIT_TEST(checkMultiValueAtISet);
        CPPUNIT_TEST(checkSimpleItems);
        CPPUNIT_TEST(checkSimpleItemsAtISet);
        CPPUNIT_TEST_SUITE_END();
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(Item::ibase);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
