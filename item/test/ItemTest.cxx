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
#include <item/base/ItemControlBlock.hxx>
#include <item/simple/CntInt16.hxx>
#include <item/simple/CntOUString.hxx>

namespace Item
{
    // example for simple multi value Item
    class MultiValueSimple : public ItemBase
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueSimple(); },
                "MultiValueSimple");

            return aItemControlBlock;
        }

    private:
        sal_Int16 m_nValueA;
        sal_Int32 m_nValueB;

    protected:
        MultiValueSimple(ItemControlBlock& rItemControlBlock, sal_Int16 nValA, sal_Int32 nValB)
        :   ItemBase(rItemControlBlock),
            m_nValueA(nValA),
            m_nValueB(nValB)
        {
        }

    public:
        MultiValueSimple(sal_Int16 nValA = 0, sal_Int32 nValB = 0)
        :   ItemBase(MultiValueSimple::GetStaticItemControlBlock()),
            m_nValueA(nValA),
            m_nValueB(nValB)
        {
        }

        virtual bool operator==(const ItemBase& rRef) const
        {
            return ItemBase::operator==(rRef) || // ptr-compare
                (getValueA() == static_cast<const MultiValueSimple&>(rRef).getValueA() &&
                 getValueB() == static_cast<const MultiValueSimple&>(rRef).getValueB());
        }

        virtual std::unique_ptr<ItemBase> clone() const
        {
            // use direct value(s) and std::make_unique
            return std::make_unique<MultiValueSimple>(getValueA(), getValueB());
        }

        sal_Int16 getValueA() const
        {
            return m_nValueA;
        }

        sal_Int32 getValueB() const
        {
            return m_nValueB;
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for derived simple multi value Item to get another
    // class identifier
    class MultiValueSimple_derivedClass : public MultiValueSimple
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueSimple_derivedClass(); },
                "MultiValueSimple_derivedClass");

            return aItemControlBlock;
        }

    public:
        MultiValueSimple_derivedClass(sal_Int16 nValA = 0, sal_Int32 nValB = 0)
        :   MultiValueSimple(MultiValueSimple_derivedClass::GetStaticItemControlBlock(), nValA, nValB)
        {
        }

        virtual std::unique_ptr<ItemBase> clone() const
        {
            // use direct value(s) and std::make_unique
            return std::make_unique<MultiValueSimple_derivedClass>(getValueA(), getValueB());
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for simple multi value Item, extended by one
    class MultiValueSimple_plus : public MultiValueSimple
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueSimple_plus(); },
                "MultiValueSimple_plus");

            return aItemControlBlock;
        }

    private:
        sal_Int64 m_nValueC;

    protected:
        MultiValueSimple_plus(ItemControlBlock& rItemControlBlock, sal_Int16 nValA, sal_Int32 nValB, sal_Int64 nValC)
        :   MultiValueSimple(rItemControlBlock, nValA, nValB),
            m_nValueC(nValC)
        {
        }

    public:
        MultiValueSimple_plus(sal_Int16 nValA = 0, sal_Int32 nValB = 0, sal_Int64 nValC = 0)
        :   MultiValueSimple(MultiValueSimple_plus::GetStaticItemControlBlock(), nValA, nValB),
            m_nValueC(nValC)
        {
        }

        virtual bool operator==(const ItemBase& rRef) const
        {
            return MultiValueSimple::operator==(rRef) || // ptr-compare && A,B
                getValueC() == static_cast<const MultiValueSimple_plus&>(rRef).getValueC();
        }

        virtual std::unique_ptr<ItemBase> clone() const
        {
            // use direct value(s) and std::make_unique
            return std::make_unique<MultiValueSimple_plus>(getValueA(), getValueB(), getValueC());
        }

        sal_Int64 getValueC() const
        {
            return m_nValueC;
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for derived simple multi value plus Item to get another
    // class identifier
    class MultiValueSimple_plus_derivedClass : public MultiValueSimple_plus
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueSimple_plus_derivedClass(); },
                "MultiValueSimple_plus_derivedClass");

            return aItemControlBlock;
        }

    public:
        MultiValueSimple_plus_derivedClass(sal_Int16 nValA = 0, sal_Int32 nValB = 0, sal_Int64 nValC = 0)
        :   MultiValueSimple_plus(MultiValueSimple_plus_derivedClass::GetStaticItemControlBlock(), nValA, nValB, nValC)
        {
        }

        virtual std::unique_ptr<ItemBase> clone() const
        {
            // use direct value(s) and std::make_unique
            return std::make_unique<MultiValueSimple_plus_derivedClass>(getValueA(), getValueB(), getValueC());
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for bufferd multi value Item
    class MultiValueBuffered : public ItemBuffered
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueBuffered(); },
                "MultiValueBuffered");

            return aItemControlBlock;
        }

    protected:
        class MultiValueData : public ItemData
        {
        private:
            sal_Int16 m_nValueA;
            sal_Int32 m_nValueB;

        protected:
            virtual ItemAdministrator& getItemAdministrator() const
            {
                static ItemAdministrator_set aItemAdministrator_set(
                    // hand over localized lambda call to construct a new instance of Item
                    [](){ return new MultiValueData(0, 0); },
                    // hand over localized lambda operator< to have a sorted set
                    [](ItemData* A, ItemData* B)
                    {
                        if(static_cast<MultiValueData*>(A)->getValueA() == static_cast<MultiValueData*>(B)->getValueA())
                        {
                            return static_cast<MultiValueData*>(A)->getValueB() < static_cast<MultiValueData*>(B)->getValueB();
                        }

                        return static_cast<MultiValueData*>(A)->getValueA() < static_cast<MultiValueData*>(B)->getValueA();
                    });
                return aItemAdministrator_set;
            }

        public:
            MultiValueData(sal_Int16 nValA = 0, sal_Int32 nValB = 0)
            :   ItemData(),
                m_nValueA(nValA),
                m_nValueB(nValB)
            {
            }

            virtual bool operator==(const ItemData& rRef) const
            {
                return ItemData::operator==(rRef) || // ptr-compare
                    (getValueA() == static_cast<const MultiValueData&>(rRef).getValueA() &&
                     getValueB() == static_cast<const MultiValueData&>(rRef).getValueB()); // content compare
            }

            sal_Int16 getValueA() const
            {
                return m_nValueA;
            }

            sal_Int32 getValueB() const
            {
                return m_nValueB;
            }
        };

    protected:
        MultiValueBuffered(ItemControlBlock& rItemControlBlock, sal_Int16 nValueA, sal_Int32 nValueB)
        :   ItemBuffered(rItemControlBlock)
        {
            setItemData(new MultiValueData(nValueA, nValueB));
        }

        MultiValueBuffered(ItemControlBlock& rItemControlBlock)
        :   ItemBuffered(rItemControlBlock)
        {
        }

    public:
        MultiValueBuffered(sal_Int16 nValueA = 0, sal_Int32 nValueB = 0)
        :   ItemBuffered(MultiValueBuffered::GetStaticItemControlBlock())
        {
            // This call initializes the values and is *required*.
            // Doing it hat way, it is possible to first
            // set values or do operations on the to-be-created
            // instance of ItemDt *before* it gets used as read-only
            // value inside this ItemBuffered incarnation
            setItemData(new MultiValueData(nValueA, nValueB));
        }

        sal_Int16 getValueA() const { return static_cast<MultiValueData const&>(getItemData()).getValueA(); }
        sal_Int32 getValueB() const { return static_cast<MultiValueData const&>(getItemData()).getValueB(); }

        void setValueA(sal_Int16 nNewA) { setItemData(new MultiValueData(nNewA, getValueB())); }
        void setValueB(sal_Int32 nNewB) { setItemData(new MultiValueData(getValueA(), nNewB)); }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for derived bufferd multi value Item to get another
    // class identifier
    class MultiValueBuffered_derivedClass : public MultiValueBuffered
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueBuffered_derivedClass(); },
                "MultiValueBuffered_derivedClass");

            return aItemControlBlock;
        }

    public:
        MultiValueBuffered_derivedClass(sal_Int16 nValueA = 0, sal_Int32 nValueB = 0)
        :   MultiValueBuffered(MultiValueBuffered_derivedClass::GetStaticItemControlBlock(), nValueA, nValueB)
        {
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for bufferd multi value Item, extended by one
    class MultiValueBuffered_plus : public MultiValueBuffered
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueBuffered_plus(); },
                "MultiValueBuffered_plus");

            return aItemControlBlock;
        }

    protected:
        class MultiValueData_plus : public MultiValueData
        {
        private:
            sal_Int64 m_nValueC;

        protected:
            virtual ItemAdministrator& getItemAdministrator() const
            {
                static ItemAdministrator_set aItemAdministrator_set(
                    // hand over localized lambda call to construct a new instance of Item
                    [](){ return new MultiValueData_plus(0, 0, 0); },
                    // hand over localized lambda operator< to have a sorted set
                    [](ItemData* A, ItemData* B)
                    {
                        if(static_cast<MultiValueData_plus*>(A)->getValueC() == static_cast<MultiValueData_plus*>(B)->getValueC())
                        {
                            // this inner part maybe reused from the MultiValueData ItemAdministrator_set
                            // which is constructed above in the class we derive from. That would need
                            // access to the used lambda there and remembering itz at ItemAdministrator at
                            // all - currently just forwarded to std::set stuff. Thus - for now - just
                            // completely formulate the operator< here
                            if(static_cast<MultiValueData*>(A)->getValueA() == static_cast<MultiValueData*>(B)->getValueA())
                            {
                                return static_cast<MultiValueData*>(A)->getValueB() < static_cast<MultiValueData*>(B)->getValueB();
                            }

                            return static_cast<MultiValueData*>(A)->getValueA() < static_cast<MultiValueData*>(B)->getValueA();
                        }

                        return static_cast<MultiValueData_plus*>(A)->getValueC() < static_cast<MultiValueData_plus*>(B)->getValueC();
                    });
                return aItemAdministrator_set;
            }

        public:
            MultiValueData_plus(sal_Int16 nValA = 0, sal_Int32 nValB = 0, sal_Int64 nValC = 0)
            :   MultiValueData(nValA, nValB),
                m_nValueC(nValC)
            {
            }

            virtual bool operator==(const ItemData& rRef) const
            {
                return MultiValueData::operator==(rRef) || // ptr-compare && A, B
                    getValueC() == static_cast<const MultiValueData_plus&>(rRef).getValueC(); // content compare
            }

            sal_Int64 getValueC() const
            {
                return m_nValueC;
            }
        };

    protected:
        MultiValueBuffered_plus(ItemControlBlock& rItemControlBlock, sal_Int16 nValueA, sal_Int32 nValueB, sal_Int64 nValueC)
        :   MultiValueBuffered(rItemControlBlock)
        {
            // This call initializes the values and is *required*.
            setItemData(new MultiValueData_plus(nValueA, nValueB, nValueC));
        }

    public:
        MultiValueBuffered_plus(sal_Int16 nValueA = 0, sal_Int32 nValueB = 0, sal_Int64 nValueC = 0)
        :   MultiValueBuffered(MultiValueBuffered_plus::GetStaticItemControlBlock())
        {
            // This call initializes the values and is *required*.
            setItemData(new MultiValueData_plus(nValueA, nValueB, nValueC));
        }

        sal_Int64 getValueC() const { return static_cast<MultiValueData_plus const&>(getItemData()).getValueC(); }

        void setValueC(sal_Int64 nNewC) { setItemData(new MultiValueData_plus(getValueA(), getValueB(), nNewC)); }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for derived bufferd multi value plus Item to get another
    // class identifier
    class MultiValueBuffered_plus_derivedClass : public MultiValueBuffered_plus
    {
    public:
        static ItemControlBlock& GetStaticItemControlBlock()
        {
            static ItemControlBlock aItemControlBlock(
                [](){ return new MultiValueBuffered_plus_derivedClass(); },
                "MultiValueBuffered_plus_derivedClass");

            return aItemControlBlock;
        }

    public:
        MultiValueBuffered_plus_derivedClass(sal_Int16 nValueA = 0, sal_Int32 nValueB = 0, sal_Int64 nValueC = 0)
        :   MultiValueBuffered_plus(MultiValueBuffered_plus_derivedClass::GetStaticItemControlBlock(), nValueA, nValueB, nValueC)
        {
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ibase : public CppUnit::TestFixture
    {
    private:
    public:
        void checkMultiValueSimple()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueSimple a_sp, b_sp, c_sp;
            static MultiValueSimple theDefault;

            a_sp = MultiValueSimple(3, 3);
            b_sp = MultiValueSimple(5, 5);
            c_sp = MultiValueSimple(7, 7);
            theDefault = Item::getDefault<MultiValueSimple>();

            MultiValueSimple mhas3(MultiValueSimple(5, 5));
            MultiValueSimple mhas4(MultiValueSimple(2, 2));

            if(MultiValueSimple(3, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueSimple(0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueSimple testAA(11, 22);
            std::vector<MultiValueSimple> testA;
            std::vector<MultiValueSimple> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueSimple(aloop+1, (aloop+1)*2));
                testB.push_back(MultiValueSimple(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2)));
            }

            MultiValueSimple hasA(MultiValueSimple(2, 4));
            MultiValueSimple hasB(MultiValueSimple(2, 3));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueSimple hasC(testA.front());
            MultiValueSimple hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValueSimple_derivedClass()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueSimple_derivedClass a_sp, b_sp, c_sp;
            static MultiValueSimple_derivedClass theDefault;

            a_sp = MultiValueSimple_derivedClass(3, 3);
            b_sp = MultiValueSimple_derivedClass(5, 5);
            c_sp = MultiValueSimple_derivedClass(7, 7);
            theDefault = Item::getDefault<MultiValueSimple_derivedClass>();

            MultiValueSimple_derivedClass mhas3(MultiValueSimple_derivedClass(5, 5));
            MultiValueSimple_derivedClass mhas4(MultiValueSimple_derivedClass(2, 2));

            if(MultiValueSimple_derivedClass(3, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueSimple_derivedClass(0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueSimple_derivedClass testAA(11, 22);
            std::vector<MultiValueSimple_derivedClass> testA;
            std::vector<MultiValueSimple_derivedClass> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueSimple_derivedClass(aloop+1, (aloop+1)*2));
                testB.push_back(MultiValueSimple_derivedClass(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2)));
            }

            MultiValueSimple_derivedClass hasA(MultiValueSimple_derivedClass(2, 4));
            MultiValueSimple_derivedClass hasB(MultiValueSimple_derivedClass(2, 3));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueSimple_derivedClass hasC(testA.front());
            MultiValueSimple_derivedClass hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValueSimple_plus()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueSimple_plus a_sp, b_sp, c_sp;
            static MultiValueSimple_plus theDefault;

            a_sp = MultiValueSimple_plus(3, 3, 3);
            b_sp = MultiValueSimple_plus(5, 5, 5);
            c_sp = MultiValueSimple_plus(7, 7, 7);
            theDefault = Item::getDefault<MultiValueSimple_plus>();

            MultiValueSimple_plus mhas3(MultiValueSimple_plus(5, 5, 5));
            MultiValueSimple_plus mhas4(MultiValueSimple_plus(2, 2, 2));

            if(MultiValueSimple_plus(3, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueSimple_plus(0, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueSimple_plus testAA(11, 22, 33);
            std::vector<MultiValueSimple_plus> testA;
            std::vector<MultiValueSimple_plus> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueSimple_plus(aloop+1, (aloop+1)*2, (aloop+1)*3));
                testB.push_back(MultiValueSimple_plus(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2), nLoopNumber-((aloop+1)*3)));
            }

            MultiValueSimple_plus hasA(MultiValueSimple_plus(2, 4, 6));
            MultiValueSimple_plus hasB(MultiValueSimple_plus(2, 3, 4));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueSimple_plus hasC(testA.front());
            MultiValueSimple_plus hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValueSimple_plus_derivedClass()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueSimple_plus_derivedClass a_sp, b_sp, c_sp;
            static MultiValueSimple_plus_derivedClass theDefault;

            a_sp = MultiValueSimple_plus_derivedClass(3, 3, 3);
            b_sp = MultiValueSimple_plus_derivedClass(5, 5, 5);
            c_sp = MultiValueSimple_plus_derivedClass(7, 7, 7);
            theDefault = Item::getDefault<MultiValueSimple_plus_derivedClass>();

            MultiValueSimple_plus_derivedClass mhas3(MultiValueSimple_plus_derivedClass(5, 5, 5));
            MultiValueSimple_plus_derivedClass mhas4(MultiValueSimple_plus_derivedClass(2, 2, 2));

            if(MultiValueSimple_plus_derivedClass(3, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueSimple_plus_derivedClass(0, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueSimple_plus_derivedClass testAA(11, 22, 33);
            std::vector<MultiValueSimple_plus_derivedClass> testA;
            std::vector<MultiValueSimple_plus_derivedClass> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueSimple_plus_derivedClass(aloop+1, (aloop+1)*2, (aloop+1)*3));
                testB.push_back(MultiValueSimple_plus_derivedClass(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2), nLoopNumber-((aloop+1)*3)));
            }

            MultiValueSimple_plus_derivedClass hasA(MultiValueSimple_plus_derivedClass(2, 4, 6));
            MultiValueSimple_plus_derivedClass hasB(MultiValueSimple_plus_derivedClass(2, 3, 4));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueSimple_plus_derivedClass hasC(testA.front());
            MultiValueSimple_plus_derivedClass hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValueBuffered()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueBuffered a_sp, b_sp, c_sp;
            static MultiValueBuffered theDefault;

            a_sp = MultiValueBuffered(3, 3);
            b_sp = MultiValueBuffered(5, 5);
            c_sp = MultiValueBuffered(7, 7);
            theDefault = Item::getDefault<MultiValueBuffered>();

            MultiValueBuffered mhas3(MultiValueBuffered(5, 5));
            MultiValueBuffered mhas4(MultiValueBuffered(2, 2));

            if(MultiValueBuffered(3, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueBuffered(0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueBuffered testAA(11, 22);
            std::vector<MultiValueBuffered> testA;
            std::vector<MultiValueBuffered> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueBuffered(aloop+1, (aloop+1)*2));
                testB.push_back(MultiValueBuffered(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2)));
            }

            MultiValueBuffered hasA(MultiValueBuffered(2, 4));
            MultiValueBuffered hasB(MultiValueBuffered(2, 3));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueBuffered hasC(testA.front());
            MultiValueBuffered hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValueBuffered_derivedClass()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueBuffered_derivedClass a_sp, b_sp, c_sp;
            static MultiValueBuffered_derivedClass theDefault;

            a_sp = MultiValueBuffered_derivedClass(3, 3);
            b_sp = MultiValueBuffered_derivedClass(5, 5);
            c_sp = MultiValueBuffered_derivedClass(7, 7);
            theDefault = Item::getDefault<MultiValueBuffered_derivedClass>();

            MultiValueBuffered_derivedClass mhas3(MultiValueBuffered_derivedClass(5, 5));
            MultiValueBuffered_derivedClass mhas4(MultiValueBuffered_derivedClass(2, 2));

            if(MultiValueBuffered_derivedClass(3, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueBuffered_derivedClass(0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueBuffered_derivedClass testAA(11, 22);
            std::vector<MultiValueBuffered_derivedClass> testA;
            std::vector<MultiValueBuffered_derivedClass> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueBuffered_derivedClass(aloop+1, (aloop+1)*2));
                testB.push_back(MultiValueBuffered_derivedClass(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2)));
            }

            MultiValueBuffered_derivedClass hasA(MultiValueBuffered_derivedClass(2, 4));
            MultiValueBuffered_derivedClass hasB(MultiValueBuffered_derivedClass(2, 3));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueBuffered_derivedClass hasC(testA.front());
            MultiValueBuffered_derivedClass hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValueBuffered_plus()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueBuffered_plus a_sp, b_sp, c_sp;
            static MultiValueBuffered_plus theDefault;

            a_sp = MultiValueBuffered_plus(3, 3, 3);
            b_sp = MultiValueBuffered_plus(5, 5, 5);
            c_sp = MultiValueBuffered_plus(7, 7, 7);
            theDefault = Item::getDefault<MultiValueBuffered_plus>();

            MultiValueBuffered_plus mhas3(MultiValueBuffered_plus(5, 5, 5));
            MultiValueBuffered_plus mhas4(MultiValueBuffered_plus(2, 2, 2));

            if(MultiValueBuffered_plus(3, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueBuffered_plus(0, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueBuffered_plus testAA(11, 22, 33);
            std::vector<MultiValueBuffered_plus> testA;
            std::vector<MultiValueBuffered_plus> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueBuffered_plus(aloop+1, (aloop+1)*2, (aloop+1)*3));
                testB.push_back(MultiValueBuffered_plus(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2), nLoopNumber-((aloop+1)*3)));
            }

            MultiValueBuffered_plus hasA(MultiValueBuffered_plus(2, 4, 6));
            MultiValueBuffered_plus hasB(MultiValueBuffered_plus(2, 3, 4));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueBuffered_plus hasC(testA.front());
            MultiValueBuffered_plus hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValueBuffered_plus_derivedClass()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            int nIncrement(0);
            static MultiValueBuffered_plus_derivedClass a_sp, b_sp, c_sp;
            static MultiValueBuffered_plus_derivedClass theDefault;

            a_sp = MultiValueBuffered_plus_derivedClass(3, 3, 3);
            b_sp = MultiValueBuffered_plus_derivedClass(5, 5, 5);
            c_sp = MultiValueBuffered_plus_derivedClass(7, 7, 7);
            theDefault = Item::getDefault<MultiValueBuffered_plus_derivedClass>();

            MultiValueBuffered_plus_derivedClass mhas3(MultiValueBuffered_plus_derivedClass(5, 5, 5));
            MultiValueBuffered_plus_derivedClass mhas4(MultiValueBuffered_plus_derivedClass(2, 2, 2));

            if(MultiValueBuffered_plus_derivedClass(3, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(MultiValueBuffered_plus_derivedClass(0, 0, 0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            MultiValueBuffered_plus_derivedClass testAA(11, 22, 33);
            std::vector<MultiValueBuffered_plus_derivedClass> testA;
            std::vector<MultiValueBuffered_plus_derivedClass> testB;
            const sal_uInt32 nLoopNumber(50);

            for(sal_uInt32 aloop(0); aloop < nLoopNumber; aloop++)
            {
                testA.push_back(MultiValueBuffered_plus_derivedClass(aloop+1, (aloop+1)*2, (aloop+1)*3));
                testB.push_back(MultiValueBuffered_plus_derivedClass(nLoopNumber-(aloop+1), nLoopNumber-((aloop+1)*2), nLoopNumber-((aloop+1)*3)));
            }

            MultiValueBuffered_plus_derivedClass hasA(MultiValueBuffered_plus_derivedClass(2, 4, 6));
            MultiValueBuffered_plus_derivedClass hasB(MultiValueBuffered_plus_derivedClass(2, 3, 4));

            testA.insert(testA.end(), testB.begin(), testB.end());
            testB.clear();

            MultiValueBuffered_plus_derivedClass hasC(testA.front());
            MultiValueBuffered_plus_derivedClass hasD(testA.back());

            while(testA.size() > nLoopNumber / 10)
            {
                testA.erase(testA.begin() + (testA.size() / 2));
            }

            testA.clear();
        }

        void checkMultiValuesAtISet()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

           int nIncrement(0);

            // make use of local MultiValueAB item in conjuction with ItemSet
            ModelSpecificItemValues::SharedPtr aModelSpecificIValues(ModelSpecificItemValues::create());
            ItemSet::SharedPtr aSet(ItemSet::create(aModelSpecificIValues));

            ///////////////////////////////////////////////////////////////////////////////

            MultiValueSimple aSimple(2,2);
            MultiValueSimple_derivedClass aSimple_derivedClass(3,3);
            MultiValueSimple_plus aSimple_plus(4,4,4);
            MultiValueSimple_plus_derivedClass aSimple_plus_derivedClass(5,5,5);
            MultiValueBuffered aBuffered(6,6);
            MultiValueBuffered_derivedClass aBuffered_derivedClass(7,7);
            MultiValueBuffered_plus aBuffered_plus(8,8,8);
            MultiValueBuffered_plus_derivedClass aBuffered_plus_derivedClass(9,9,9);

            aSet->setItem(aSimple);
            aSet->setItem(aSimple_derivedClass);
            aSet->setItem(aSimple_plus);
            aSet->setItem(aSimple_plus_derivedClass);
            aSet->setItem(aBuffered);
            aSet->setItem(aBuffered_derivedClass);
            aSet->setItem(aBuffered_plus);
            aSet->setItem(aBuffered_plus_derivedClass);

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueSimple>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueSimple_derivedClass>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueSimple_plus>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueSimple_plus_derivedClass>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueBuffered>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueBuffered_derivedClass>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueBuffered_plus>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            ///////////////////////////////////////////////////////////////////////////////

            if(const auto Item(aSet->getStateAndItem<MultiValueBuffered_plus_derivedClass>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            // CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
            //                              B2IRange(1, 2, 4, 5), fround(ibase(1.2, 2.3, 3.5, 4.8)));
        }

        void checkMultiValuesAtISetMixed()
        {
            // for debug, change bLoop to true, start, attach and set to false again to debug (one possibility...)
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

           int nIncrement(0);

            // make use of local MultiValueAB item in conjuction with ItemSet
            ModelSpecificItemValues::SharedPtr aModelSpecificIValues(ModelSpecificItemValues::create());
            ItemSet::SharedPtr aSet(ItemSet::create(aModelSpecificIValues));

            ///////////////////////////////////////////////////////////////////////////////

            MultiValueSimple aSimple(2,2);
            MultiValueSimple_derivedClass aSimple_derivedClass(3,3);
            MultiValueSimple_plus aSimple_plus(4,4,4);
            MultiValueSimple_plus_derivedClass aSimple_plus_derivedClass(5,5,5);
            MultiValueBuffered aBuffered(6,6);
            MultiValueBuffered_derivedClass aBuffered_derivedClass(7,7);
            MultiValueBuffered_plus aBuffered_plus(8,8,8);
            MultiValueBuffered_plus_derivedClass aBuffered_plus_derivedClass(9,9,9);

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueSimple>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueSimple>());
                aSet->clearItem<MultiValueSimple>();
                const auto ItemC(aSet->getStateAndItem<MultiValueSimple>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueSimple_derivedClass>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueSimple_derivedClass>());
                aSet->clearItem<MultiValueSimple_derivedClass>();
                const auto ItemC(aSet->getStateAndItem<MultiValueSimple_derivedClass>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueSimple_plus>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueSimple_plus>());
                aSet->clearItem<MultiValueSimple_plus>();
                const auto ItemC(aSet->getStateAndItem<MultiValueSimple_plus>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueSimple_plus_derivedClass>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueSimple_plus_derivedClass>());
                aSet->clearItem<MultiValueSimple_plus_derivedClass>();
                const auto ItemC(aSet->getStateAndItem<MultiValueSimple_plus_derivedClass>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueBuffered>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueBuffered>());
                aSet->clearItem<MultiValueBuffered>();
                const auto ItemC(aSet->getStateAndItem<MultiValueBuffered>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueBuffered_derivedClass>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueBuffered_derivedClass>());
                aSet->clearItem<MultiValueBuffered_derivedClass>();
                const auto ItemC(aSet->getStateAndItem<MultiValueBuffered_derivedClass>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueBuffered_plus>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueBuffered_plus>());
                aSet->clearItem<MultiValueBuffered_plus>();
                const auto ItemC(aSet->getStateAndItem<MultiValueBuffered_plus>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueBuffered_plus_derivedClass>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueBuffered_plus_derivedClass>());
                aSet->clearItem<MultiValueBuffered_plus_derivedClass>();
                const auto ItemC(aSet->getStateAndItem<MultiValueBuffered_plus_derivedClass>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////

            {
                const auto ItemA(aSet->getStateAndItem<MultiValueSimple>());
                aSet->setItem(aSimple);
                const auto ItemB(aSet->getStateAndItem<MultiValueSimple>());
                aSet->clearItem<MultiValueSimple>();
                const auto ItemC(aSet->getStateAndItem<MultiValueSimple>());

                if(ItemC.isSet())
                {
                    nIncrement++;
                }
            }

            // CPPUNIT_ASSERT_EQUAL_MESSAGE("simple range rounding from double to integer",
            //                              B2IRange(1, 2, 4, 5), fround(ibase(1.2, 2.3, 3.5, 4.8)));
        }

        void checkSimpleItems()
        {
            static bool bInit(false);
            static CntInt16 a_sp, b_sp, c_sp;
            static CntInt16 theDefault;
            static CntOUString sa_sp, sb_sp, sc_sp;
            static CntOUString stheDefault;
            const sal_uInt32 nLoopNumber(50);
            int nIncrement(0);

            if(!bInit)
            {
                bInit = true;
                a_sp = CntInt16(3);
                b_sp = CntInt16(5);
                c_sp = CntInt16(7);
                theDefault = Item::getDefault<CntInt16>();

                sa_sp = CntOUString("Hello");
                sb_sp = CntOUString("World");
                sc_sp = CntOUString("..of Doom!");
                stheDefault = Item::getDefault<CntOUString>();
            }

            ///////////////////////////////////////////////////////////////////////////////

            CntInt16 has3(CntInt16(3));
            CntInt16 has4(CntInt16(4));

            if(CntInt16(11).isDefault())
            {
                nIncrement++;
            }

            if(CntInt16(0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            std::vector<CntInt16> test16;

            for(sal_uInt32 a(0); a < nLoopNumber; a++)
            {
                test16.push_back(CntInt16(a));
            }

            test16.clear();

            ///////////////////////////////////////////////////////////////////////////////

            CntOUString shas3(CntOUString("Hello"));
            CntOUString shas4(CntOUString("WhateverComesAlong"));

            if(CntOUString("NotDefault").isDefault())
            {
                nIncrement++;
            }

            if(CntOUString(OUString()).isDefault())
            {
                nIncrement++;
            }

            if(stheDefault.isDefault())
            {
                nIncrement++;
            }

            std::vector<CntOUString> testStr;

            for(sal_uInt32 a(0); a < nLoopNumber; a++)
            {
                testStr.push_back(CntOUString(OUString::number(static_cast<int>(a))));
            }

            testStr.clear();

            if(bInit)
            {
                bInit = false;
            }
        }

        void checkSimpleItemsAtISet()
        {
            int nIncrement(0);

            ModelSpecificItemValues::SharedPtr aModelSpecificIValues(ModelSpecificItemValues::create());
            aModelSpecificIValues->setAlternativeDefaultItem(CntInt16(3));
            aModelSpecificIValues->setAlternativeDefaultItem(CntInt16(4));

            ItemSet::SharedPtr aSet(ItemSet::create(aModelSpecificIValues));
            const auto aActEmpty(aSet->getStateAndItem<CntInt16>());

            aSet->setItem(CntInt16(4));
            const auto aActA(aSet->getStateAndItem<CntInt16>());

            aSet->setItem(Item::getDefault<CntInt16>());
            const auto aActB(aSet->getStateAndItem<CntInt16>());

            aSet->setItem(CntInt16(12));
            const auto aActC(aSet->getStateAndItem<CntInt16>());

            aSet->setItem(CntOUString("Teststring - not really useful :-)"));
            const auto aActStr(aSet->getStateAndItem<CntOUString>());

            const auto ItemDADA(aSet->getStateAndItem<CntInt16>());

            if(const auto Item(aSet->getStateAndItem<CntOUString>()); Item.isSet())
            {
                nIncrement += (ItemSet::IState::SET == Item.getIState()) ? 1 : 0;
                nIncrement += Item.isDisabled();
            }
            else if(Item.isDefault())
            {
                nIncrement++;
            }
            else if(Item.isDontCare())
            {
                nIncrement++;
            }
            else if(Item.isDisabled())
            {
                nIncrement++;
            }

            // check getting default at Set, this will include ModelSpecificItemValues
            // compared with the static ItemBase::GetDefault<CntInt16>() call
            const CntInt16 aDefSet(aSet->getDefault<CntInt16>());
            const CntInt16 aDefGlobal(Item::getDefault<CntInt16>());

            const bool bClA(aSet->clearItem<const CntInt16>());
            nIncrement += bClA;
            // let one exist to check destruction when Set gets destructed
            // const bool bClB(aSet->ClearItem<const CntOUString>());
            nIncrement ++;
        }

        void checkItem2()
        {
            static bool bLoop(false);
            while(bLoop)
            {
                bLoop = true;
            }

            static bool bInit(false);
            static CntInt16 a_sp, b_sp, c_sp;
            static CntInt16 theDefault;
            const sal_uInt32 nLoopNumber(50);
            int nIncrement(0);

            if(!bInit)
            {
                bInit = true;
                a_sp = CntInt16(3);
                b_sp = CntInt16(5);
                c_sp = CntInt16(7);
                theDefault = Item::getDefault<CntInt16>();
            }

            ///////////////////////////////////////////////////////////////////////////////

            CntInt16 has3(CntInt16(3));
            CntInt16 has4(CntInt16(4));

            if(CntInt16(11).isDefault())
            {
                nIncrement++;
            }

            if(CntInt16(0).isDefault())
            {
                nIncrement++;
            }

            if(theDefault.isDefault())
            {
                nIncrement++;
            }

            std::vector<CntInt16> test16;

            for(sal_uInt32 a(0); a < nLoopNumber; a++)
            {
                test16.push_back(CntInt16(a));
            }

            test16.clear();

            if(bInit)
            {
                bInit = false;
            }
        }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(ibase);
        CPPUNIT_TEST(checkMultiValueSimple);
        CPPUNIT_TEST(checkMultiValueSimple_derivedClass);
        CPPUNIT_TEST(checkMultiValueSimple_plus);
        CPPUNIT_TEST(checkMultiValueSimple_plus_derivedClass);
        CPPUNIT_TEST(checkMultiValueBuffered);
        CPPUNIT_TEST(checkMultiValueBuffered_derivedClass);
        CPPUNIT_TEST(checkMultiValueBuffered_plus);
        CPPUNIT_TEST(checkMultiValueBuffered_plus_derivedClass);
        CPPUNIT_TEST(checkMultiValuesAtISet);
        CPPUNIT_TEST(checkMultiValuesAtISetMixed);
        CPPUNIT_TEST(checkSimpleItems);
        CPPUNIT_TEST(checkSimpleItemsAtISet);
        CPPUNIT_TEST(checkItem2);
        CPPUNIT_TEST_SUITE_END();
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(Item::ibase);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
