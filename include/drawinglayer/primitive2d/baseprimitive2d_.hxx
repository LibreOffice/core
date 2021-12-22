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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/CommonTypes.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/util/XAccounting.hpp>

namespace drawinglayer::primitive2d
{
    /// c++ implementation typedefs & clases for Primitive2D_SharedPtr,
    /// Primitive2D_Vector && Primitive2D_VectorSharedPtr
    class BasePrimitive2D_;

    /// Hold a single BasePrimitive2D shared. This should
    /// never be empty by definition
    typedef std::shared_ptr< BasePrimitive2D_ > Primitive2D_SharedPtr;

    /// Use a std::vector to hold an array of shared BasePrimitive2D
    /// Note: I thought here about having a simple std::vector< BasePrimitive2D >
    ///       which then could be extended to a small helper class to do the
    ///       needed destruction stuff and other, but C++ std::shared_ptr
    ///       urges to do differently, please look at
    ///       https://en.cppreference.com/w/cpp/memory/enable_shared_from_this
    /// The main problem is that ::shared_from_this only works correctly when
    /// already a shared_ptr to that object exists - it does not create the shared
    /// ptr control block, but just look for it.
    /// This forces to have a protected (due to derived classes, else private)
    /// consructor and a factory function create(). With that combination
    /// it is safe that always a shared_ptr is incarnmated with the object. For
    /// that reason it is also necessary to have a std::vector< Primitive2D_SharedPtr >
    /// for this definition
    typedef std::vector< Primitive2D_SharedPtr > Primitive2D_Vector;

    /// Hold a whole BasePrimitive2D Primitive2D_VectorSharedPtr shared.
    /// This can be empty expressing that there is no content. In that way,
    /// offers a non-expensive way to handle stuff when a Primitive2D_Vector
    /// is not needed
    typedef std::shared_ptr< Primitive2D_Vector > Primitive2D_VectorSharedPtr;

    /// Base implementation of BaPr2D, prepared for shared_ptr
    /// Note: See above link & explanation to std::shared_ptr how this is intended
    ///       and used
    //
    // example test code used to check this version of BasePrimitive2D_:
    // {
    //     drawinglayer::primitive2d::Primitive2D_SharedPtr aShared(drawinglayer::primitive2d::TestPrimitive2D::create());
    //     ::css::uno::Reference<::css::graphic::XPrimitive2D> XPrimitive(aShared->getUnoPrimitive2D());
    //     drawinglayer::primitive2d::Primitive2D_SharedPtr pBackA(drawinglayer::primitive2d::TestPrimitive2D::getKnownImplementation(XPrimitive));
    //     aShared.reset();
    //     drawinglayer::primitive2d::Primitive2D_SharedPtr pBackB(drawinglayer::primitive2d::TestPrimitive2D::getKnownImplementation(XPrimitive));
    // }

    // {
    //     drawinglayer::primitive2d::Primitive2D_SharedPtr aShared(drawinglayer::primitive2d::TestPrimitive2D::create());
    //     ::css::uno::Reference<::css::graphic::XPrimitive2D> XPrimitive(aShared->getUnoPrimitive2D());

    //     drawinglayer::geometry::ViewInformation2D aDummy;
    //     const drawinglayer::primitive2d::Primitive2D_VectorSharedPtr& rResA(aShared->getB2DDecomposition(aDummy));
    //     basegfx::B2DRange aResB(aShared->getB2DRange(aDummy));

    //     css::uno::Sequence<css::beans::PropertyValue> aViewParameters;
    //     css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>> aResC(XPrimitive->getDecomposition(aViewParameters));
    //     css::geometry::RealRectangle2D aResD(XPrimitive->getRange(aViewParameters));
    // }
    //
    class DRAWINGLAYER_DLLPUBLIC BasePrimitive2D_ : public std::enable_shared_from_this< BasePrimitive2D_ >
    {
        /// no assignment or copy-construction
        BasePrimitive2D_(const BasePrimitive2D_&) = delete;
        BasePrimitive2D_& operator=(const BasePrimitive2D_&) = delete;

        /// UNO API helper class, internal, private to BasePrimitive2D
        /// Re-uses already existing class from earlier version AFAP
        class BasePrimitive2DImplBase_
        :   public cppu::OWeakObject,
            public css::lang::XComponent,
            public css::lang::XTypeProvider,
            public css::graphic::XPrimitive2D,
            public css::util::XAccounting
        {
            /// simple back-reference to BasePrimitive2D data object
            drawinglayer::primitive2d::BasePrimitive2D_* m_pSource;

        public:
            BasePrimitive2DImplBase_(drawinglayer::primitive2d::BasePrimitive2D_& rSource);
            virtual ~BasePrimitive2DImplBase_() override;

            virtual void SAL_CALL acquire() noexcept override;
            virtual void SAL_CALL release() noexcept override;
            virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const& aType) override;

            // css::lang::XComponent
            virtual void SAL_CALL dispose() override;
            virtual void SAL_CALL addEventListener(css::uno::Reference<css::lang::XEventListener> const& xListener) override;
            virtual void SAL_CALL removeEventListener(css::uno::Reference<css::lang::XEventListener> const& xListener) override;

            // css::lang::XTypeProvider
            virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override;
            virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

            // Methods from XPrimitive2D
            virtual css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>> SAL_CALL
                getDecomposition(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters) override;
            virtual css::geometry::RealRectangle2D SAL_CALL
                getRange(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters) override;

            // XAccounting
            virtual sal_Int64 SAL_CALL estimateUsage() override;

            // allow local access from inside BasePrimitive2D implementations
            drawinglayer::primitive2d::BasePrimitive2D_* getKnownImplementation() const { return m_pSource; }
        };

        /// on-demand access to UNO API implementation
        BasePrimitive2DImplBase_* m_pImplBase;

    protected:
        // Constructor is protected to force to work with shared_ptr, see Notes above
        BasePrimitive2D_();

    public:
        // No public constructor, only a factory function, so there's no way to have
        // getptr return nullptr.
        // See notes above. Commented due to BasePrimitive2D beingh pure virtual. Not
        // deleted for documentation purposes.
        // static Primitive2D_SharedPtr create()
        // {
        //     return std::shared_ptr<BasePrimitive2D_>(new BasePrimitive2D_());
        // }

        // destructor
        virtual ~BasePrimitive2D_();

        // create a shared_ptr that shares this instance guaranteed. This method can be used
        // by all derivations and instances of BasePrimitive2D
        Primitive2D_SharedPtr getptr()
        {
            return shared_from_this();
        }

        /// access to on-demand constructed Uno Impementation
        ::css::uno::Reference<::css::graphic::XPrimitive2D> getUnoPrimitive2D() const;

        /** the ==operator is mainly needed to allow testing newly-created primitives against their last
            incarnation which buffers/holds the made decompositions. The default implementation
            uses getPrimitive2DID()-calls to test if it's the same ID at last.
            Overridden implementations are then based on this implementation
         */
        virtual bool operator==(const BasePrimitive2D_& rPrimitive) const;
        bool operator!=(const BasePrimitive2D_& rPrimitive) const { return !operator==(rPrimitive); }

        /** provide unique ID for fast identifying of known primitive implementations in renderers. These use
            the defines from drawinglayer_primitivetypes2d.hxx to define unique IDs.
         */
        virtual sal_uInt32 getPrimitive2DID() const = 0;

        // C++ local Methods from XPrimitive2D
        virtual const Primitive2D_VectorSharedPtr& getB2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const = 0;

        virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const = 0;

        // C++ local Methods from XAccounting
        virtual sal_Int64 estimateUsage() const;

        /// Try to get back to implementation starting at uno reference
        /// This will work as long as the data object BasePrimitive2D exists and
        /// return the safe shared_ptr to keep the single shared context upright.
        static Primitive2D_SharedPtr getKnownImplementation(const ::css::uno::Reference<::css::graphic::XPrimitive2D>& rSource);
    };

    class DRAWINGLAYER_DLLPUBLIC TestPrimitive2D : public BasePrimitive2D_
    {
        Primitive2D_VectorSharedPtr m_aDecomposition;

    protected:
        // constructor is protected to force to work with shared_ptr
        TestPrimitive2D();

    public:
        // No public constructor, only a factory function, so there's no way to have getptr return nullptr.
        static Primitive2D_SharedPtr create()
        {
            return std::shared_ptr<TestPrimitive2D>(new TestPrimitive2D());
        }

        virtual ~TestPrimitive2D() override;

        virtual bool operator==(const BasePrimitive2D_& rPrimitive) const override;

        virtual sal_uInt32 getPrimitive2DID() const override;

        virtual const Primitive2D_VectorSharedPtr& getB2DDecomposition(
            const geometry::ViewInformation2D& rViewInformation) const override;

        virtual basegfx::B2DRange getB2DRange(
            const geometry::ViewInformation2D& rViewInformation) const override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
