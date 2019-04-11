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
#include <item/base/ItemControlBlock.hxx>

namespace Item
{
    // example for multi value Item
    // if this should be based on faster IAdministrator_set, changes needed would be:
    // - typedef ItemBaseStaticHelper<MultiValueAB, IAdministrator_set> MultiValueStaticHelperAB;
    // - define virtual bool operator<(const ItemBase& rCandidate) const override
    class MultiValueAB : public ItemBase
    {
    public:
        static ItemControlBlock& MultiValueAB::GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                std::shared_ptr<ItemAdministrator>(new IAdministrator_vector()),
                [](){ return new MultiValueAB(MultiValueAB::GetStaticItemControlBlock()); },
                [](){ return new MultiValueAB(MultiValueAB::GetStaticItemControlBlock()); });

            return aItemControlBlock;
        }

        sal_Int16 m_nValueA;
        sal_Int32 m_nValueB;

    protected:
        MultiValueAB(
            ItemControlBlock& rItemControlBlock,
            sal_Int16 nValueA = 0,
            sal_Int16 nValueB = 0)
        :   ItemBase(rItemControlBlock),
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
                MultiValueAB::GetStaticItemControlBlock().GetItemAdministrator()->Create(
                    new MultiValueAB(
                        MultiValueAB::GetStaticItemControlBlock(),
                        nValueA,
                        nValueB)));
        }

        virtual bool operator==(const ItemBase& rCandidate) const override
        {
            if(ItemBase::operator==(rCandidate)) // compares ptrs
            {
                return true;
            }

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
    class MultiValueABC : public MultiValueAB
    {
    public:
        static ItemControlBlock& MultiValueABC::GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                std::shared_ptr<ItemAdministrator>(new IAdministrator_vector()),
                [](){ return new MultiValueABC(MultiValueABC::GetStaticItemControlBlock()); },
                [](){ return new MultiValueABC(MultiValueABC::GetStaticItemControlBlock()); });

            return aItemControlBlock;
        }

        sal_Int64 m_nValueC;

    protected:
        MultiValueABC(
            ItemControlBlock& rItemControlBlock,
            sal_Int16 nValueA = 0,
            sal_Int16 nValueB = 0,
            sal_Int16 nValueC = 0)
        :   MultiValueAB(
                rItemControlBlock,
                nValueA,
                nValueB),
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
                MultiValueABC::GetStaticItemControlBlock().GetItemAdministrator()->Create(
                    new MultiValueABC(
                        MultiValueABC::GetStaticItemControlBlock(),
                        nValueA,
                        nValueB,
                        nValueC)));
        }

        virtual bool operator==(const ItemBase& rCandidate) const override
        {
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
        //     return MultiValueAB::operator<(rCandidate)
        //         && static_cast<const MultiValueABC*>(this)->GetValueC() < static_cast<const MultiValueABC&>(rCandidate).GetValueC();
        // }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for Item deived from existing one, only new type
    class MultiValueABD : public MultiValueAB
    {
    public:
        static ItemControlBlock& MultiValueABD::GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                std::shared_ptr<ItemAdministrator>(new IAdministrator_vector()),
                [](){ return new MultiValueABD(MultiValueABD::GetStaticItemControlBlock()); },
                [](){ return new MultiValueABD(MultiValueABD::GetStaticItemControlBlock()); });

            return aItemControlBlock;
        }

    protected:
        MultiValueABD(
            ItemControlBlock& rItemControlBlock,
            sal_Int16 nValueA = 0,
            sal_Int16 nValueB = 0)
        :   MultiValueAB(
            rItemControlBlock,
            nValueA,
            nValueB)
        {
        }

    public:
        static std::shared_ptr<const MultiValueABD> Create(sal_Int16 nValueA, sal_Int16 nValueB)
        {
            return std::static_pointer_cast<const MultiValueABD>(
                MultiValueABD::GetStaticItemControlBlock().GetItemAdministrator()->Create(
                    new MultiValueABD(
                        MultiValueABD::GetStaticItemControlBlock(),
                        nValueA,
                        nValueB)));
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for Item deived from existing combined one, only new type
    class MultiValueABCD : public MultiValueABC
    {
    public:
        static ItemControlBlock& MultiValueABCD::GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                std::shared_ptr<ItemAdministrator>(new IAdministrator_vector()),
                [](){ return new MultiValueABCD(MultiValueABCD::GetStaticItemControlBlock()); },
                [](){ return new MultiValueABCD(MultiValueABCD::GetStaticItemControlBlock()); });

            return aItemControlBlock;
        }

    protected:
        MultiValueABCD(
            ItemControlBlock& rItemControlBlock,
            sal_Int16 nValueA = 0,
            sal_Int16 nValueB = 0,
            sal_Int16 nValueC = 0)
        :   MultiValueABC(
            rItemControlBlock,
            nValueA,
            nValueB,
            nValueC)
        {
        }

    public:
        static std::shared_ptr<const MultiValueABCD> Create(sal_Int16 nValueA, sal_Int16 nValueB, sal_Int16 nValueC)
        {
            return std::static_pointer_cast<const MultiValueABCD>(
                MultiValueABCD::GetStaticItemControlBlock().GetItemAdministrator()->Create(
                    new MultiValueABCD(
                        MultiValueABCD::GetStaticItemControlBlock(),
                        nValueA,
                        nValueB,
                        nValueC)));
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
        void checkMultiValueAB()
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

            std::shared_ptr<const MultiValueAB> mhas3(MultiValueAB::Create(5,2));
            std::shared_ptr<const MultiValueAB> mhas4(MultiValueAB::Create(2,3));

            if(ItemBase::IsDefault(MultiValueAB::Create(3,0)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(MultiValueAB::Create(8,7)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(ItemBase::GetDefault<MultiValueAB>()))
            {
                nIncrement++;
            }

            std::shared_ptr<const MultiValueAB> testAA(MultiValueAB::Create(11,22));
            std::vector<std::shared_ptr<const MultiValueAB>> test;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                test.push_back(MultiValueAB::Create(aloop+1, (aloop+1)*2));
            }

            std::shared_ptr<const MultiValueAB> testA(MultiValueAB::Create(2,4));
            std::shared_ptr<const MultiValueAB> testB(MultiValueAB::Create(2,3));

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

        void checkMultiValueABC()
        {
            // make direct use of local MultiValueABC item
            int nIncrement(0);

            std::shared_ptr<const MultiValueABC> mhas3x(MultiValueABC::Create(5,2,5));
            std::shared_ptr<const MultiValueABC> mhas4x(MultiValueABC::Create(2,3,7));

            if(ItemBase::IsDefault(MultiValueABC::Create(3,0,5)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(MultiValueABC::Create(8,7,12)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(ItemBase::GetDefault<MultiValueABC>()))
            {
                nIncrement++;
            }

            std::vector<std::shared_ptr<const MultiValueABC>> testx;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testx.push_back(MultiValueABC::Create(aloop+1, (aloop+1)*2, (aloop+1)*4));
            }

            std::shared_ptr<const MultiValueABC> testAx(MultiValueABC::Create(2,4,3));
            std::shared_ptr<const MultiValueABC> testBx(MultiValueABC::Create(2,4,6));

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

        void checkMultiValueABD()
        {
            int nIncrement(0);
            const sal_uInt32 nLoopNumber(50);
            std::shared_ptr<const MultiValueABD> mhas3_A(MultiValueABD::Create(5,2));

            if(ItemBase::IsDefault(MultiValueABD::Create(3,0)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(MultiValueABD::Create(8,7)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(ItemBase::GetDefault<MultiValueABD>()))
            {
                nIncrement++;
            }

            std::vector<std::shared_ptr<const MultiValueABD>> testAB_A;

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testAB_A.push_back(MultiValueABD::Create(aloop+1, (aloop+1)*2));
            }

            std::shared_ptr<const MultiValueABD> testA_A(MultiValueABD::Create(2,4));
            std::shared_ptr<const MultiValueABD> testB_A(MultiValueABD::Create(2,3));

            for(sal_uInt32 dloop(0); dloop < nLoopNumber; dloop+=2)
            {
                testAB_A[dloop] = nullptr;
                if(dloop%5)
                {
                    testAB_A.push_back(MultiValueABD::Create(dloop+1, (dloop+1)*2));
                }
            }

            for(sal_uInt32 eloop(1); eloop < (nLoopNumber * 2) / 3; eloop+=2)
            {
                testAB_A[eloop] = nullptr;
                if(eloop%7)
                {
                    testAB_A.push_back(MultiValueABD::Create(eloop+1, (eloop+1)*2));
                }
            }

            testAB_A.clear();
        }

        void checkMultiValueABCD()
        {
            int nIncrement(0);
            const sal_uInt32 nLoopNumber(50);
            std::shared_ptr<const MultiValueABCD> mhas3_A(MultiValueABCD::Create(5,2, 3));

            if(ItemBase::IsDefault(MultiValueABCD::Create(3,0, 4)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(MultiValueABCD::Create(8,7,2)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(ItemBase::GetDefault<MultiValueABCD>()))
            {
                nIncrement++;
            }

            std::vector<std::shared_ptr<const MultiValueABCD>> testAB_A;

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testAB_A.push_back(MultiValueABCD::Create(aloop+1, (aloop+1)*2, (aloop+1)*5));
            }

            std::shared_ptr<const MultiValueABCD> testA_A(MultiValueABCD::Create(2,4,7));
            std::shared_ptr<const MultiValueABCD> testB_A(MultiValueABCD::Create(2,3,3));

            for(sal_uInt32 dloop(0); dloop < nLoopNumber; dloop+=2)
            {
                testAB_A[dloop] = nullptr;
                if(dloop%5)
                {
                    testAB_A.push_back(MultiValueABCD::Create(dloop+1, (dloop+1)*2, (dloop+1)*6));
                }
            }

            for(sal_uInt32 eloop(1); eloop < (nLoopNumber * 2) / 3; eloop+=2)
            {
                testAB_A[eloop] = nullptr;
                if(eloop%7)
                {
                    testAB_A.push_back(MultiValueABCD::Create(eloop+1, (eloop+1)*2, (eloop+1)*3));
                }
            }

            testAB_A.clear();
        }

        void checkMultiValuesAtISet()
        {
            int nIncrement(0);

            // make use of local MultiValueAB item in conjuction with ItemSet
            ModelSpecificItemValues::SharedPtr aModelSpecificIValues(ModelSpecificItemValues::Create());
            ItemSet::SharedPtr aSet(ItemSet::Create(aModelSpecificIValues));

            ///////////////////////////////////////////////////////////////////////////////

            std::shared_ptr<const MultiValueAB> testAB(MultiValueAB::Create(2,4));
            std::shared_ptr<const MultiValueABC> testABC(MultiValueABC::Create(2,4,6));
            std::shared_ptr<const MultiValueABD> testABD(MultiValueABD::Create(2,4));
            std::shared_ptr<const MultiValueABCD> testABCD(MultiValueABCD::Create(2,4,6));

            aSet->SetItem(testAB);
            aSet->SetItem(testABC);
            aSet->SetItem(testABD);
            aSet->SetItem(testABCD);

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto ItemAB(aSet->GetStateAndItem<const MultiValueAB>()); ItemAB.IsSet())
            {
                nIncrement += (ItemSet::IState::SET == ItemAB.GetIState()) ? 1 : 0;
                nIncrement += (ItemAB.GetItem()) ? 1 : 0;
                nIncrement += (nullptr != ItemAB.GetItemInstance()) ? 1 : 0;
                nIncrement += ItemAB.HasItem();
                nIncrement += ItemAB.IsDisabled();
            }
            else if(ItemAB.IsDefault())
            {
                nIncrement += ItemAB.IsDisabled();
            }
            else if(ItemAB.IsDontCare())
            {
                nIncrement += ItemAB.IsDisabled();
            }
            else
            {
                nIncrement += ItemAB.IsDisabled();
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto ItemABC(aSet->GetStateAndItem<const MultiValueABC>()); ItemABC.IsSet())
            {
                nIncrement += (ItemSet::IState::SET == ItemABC.GetIState()) ? 1 : 0;
                nIncrement += (ItemABC.GetItem()) ? 1 : 0;
                nIncrement += (nullptr != ItemABC.GetItemInstance()) ? 1 : 0;
                nIncrement += ItemABC.HasItem();
                nIncrement += ItemABC.IsDisabled();
            }
            else if(ItemABC.IsDefault())
            {
                nIncrement += ItemABC.IsDisabled();
            }
            else if(ItemABC.IsDontCare())
            {
                nIncrement += ItemABC.IsDisabled();
            }
            else
            {
                nIncrement += ItemABC.IsDisabled();
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto ItemABD(aSet->GetStateAndItem<const MultiValueABD>()); ItemABD.IsSet())
            {
                nIncrement += (ItemSet::IState::SET == ItemABD.GetIState()) ? 1 : 0;
                nIncrement += (ItemABD.GetItem()) ? 1 : 0;
                nIncrement += (nullptr != ItemABD.GetItemInstance()) ? 1 : 0;
                nIncrement += ItemABD.HasItem();
                nIncrement += ItemABD.IsDisabled();
            }
            else if(ItemABD.IsDefault())
            {
                nIncrement += ItemABD.IsDisabled();
            }
            else if(ItemABD.IsDontCare())
            {
                nIncrement += ItemABD.IsDisabled();
            }
            else
            {
                nIncrement += ItemABD.IsDisabled();
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto ItemABCD(aSet->GetStateAndItem<const MultiValueABCD>()); ItemABCD.IsSet())
            {
                nIncrement += (ItemSet::IState::SET == ItemABCD.GetIState()) ? 1 : 0;
                nIncrement += (ItemABCD.GetItem()) ? 1 : 0;
                nIncrement += (nullptr != ItemABCD.GetItemInstance()) ? 1 : 0;
                nIncrement += ItemABCD.HasItem();
                nIncrement += ItemABCD.IsDisabled();
            }
            else if(ItemABCD.IsDefault())
            {
                nIncrement += ItemABCD.IsDisabled();
            }
            else if(ItemABCD.IsDontCare())
            {
                nIncrement += ItemABCD.IsDisabled();
            }
            else
            {
                nIncrement += ItemABCD.IsDisabled();
            }

            // CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
            //                              B2IRange(1, 2, 4, 5), fround(ibase(1.2, 2.3, 3.5, 4.8)));
        }

        void checkSimpleItems()
        {
            static bool bInit(false);
            static std::shared_ptr<const CntInt16> a_sp, b_sp, c_sp;
            static std::shared_ptr<const CntInt16> theDefault;
            static std::shared_ptr<const CntOUString> sa_sp, sb_sp, sc_sp;
            static std::shared_ptr<const CntOUString> stheDefault;
            const sal_uInt32 nLoopNumber(50);
            int nIncrement(0);

            if(!bInit)
            {
                bInit = true;
                a_sp = CntInt16::Create(3);
                b_sp = CntInt16::Create(5);
                c_sp = CntInt16::Create(7);
                theDefault = ItemBase::GetDefault<CntInt16>();

                sa_sp = CntOUString::Create("Hello");
                sb_sp = CntOUString::Create("World");
                sc_sp = CntOUString::Create("..of Doom!");
                stheDefault = ItemBase::GetDefault<CntOUString>();
            }

            ///////////////////////////////////////////////////////////////////////////////

            std::shared_ptr<const CntInt16> has3(CntInt16::Create(3));
            std::shared_ptr<const CntInt16> has4(CntInt16::Create(4));

            if(ItemBase::IsDefault(CntInt16::Create(11)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(CntInt16::Create(0)))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(theDefault))
            {
                nIncrement++;
            }

            std::vector<std::shared_ptr<const CntInt16>> test16;

            for(sal_uInt32 a(0); a < nLoopNumber; a++)
            {
                test16.push_back(CntInt16::Create(a));
            }

            test16.clear();

            ///////////////////////////////////////////////////////////////////////////////

            std::shared_ptr<const CntOUString> shas3(CntOUString::Create("Hello"));
            std::shared_ptr<const CntOUString> shas4(CntOUString::Create("WhateverComesAlong"));

            if(ItemBase::IsDefault(CntOUString::Create("NotDefault")))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(CntOUString::Create(OUString())))
            {
                nIncrement++;
            }

            if(ItemBase::IsDefault(stheDefault))
            {
                nIncrement++;
            }

            std::vector<std::shared_ptr<const CntOUString>> testStr;

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

            aSet->SetItem(ItemBase::GetDefault<CntInt16>());
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
            // compared with the static ItemBase::GetDefault<CntInt16>() call
            const std::shared_ptr<const CntInt16> aDefSet(aSet->GetDefault<CntInt16>());
            const std::shared_ptr<const CntInt16> aDefGlobal(ItemBase::GetDefault<CntInt16>());

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
        CPPUNIT_TEST(checkMultiValueAB);
        CPPUNIT_TEST(checkMultiValueABC);
        CPPUNIT_TEST(checkMultiValueABD);
        CPPUNIT_TEST(checkMultiValueABCD);
        CPPUNIT_TEST(checkMultiValuesAtISet);
        CPPUNIT_TEST(checkSimpleItems);
        CPPUNIT_TEST(checkSimpleItemsAtISet);
        CPPUNIT_TEST_SUITE_END();
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(Item::ibase);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
