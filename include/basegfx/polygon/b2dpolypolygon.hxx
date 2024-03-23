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

#include <ostream>
#include <vector>

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/basegfxdllapi.h>
#include <basegfx/polygon/b2dpolygon.hxx>

namespace basegfx
{
    class B2DHomMatrix;
    class ImplB2DPolyPolygon;

    class BASEGFX_DLLPUBLIC B2DPolyPolygon
    {
    private:
        o3tl::cow_wrapper<ImplB2DPolyPolygon, o3tl::ThreadSafeRefCountingPolicy> mpPolyPolygon;

    public:
        B2DPolyPolygon();
        B2DPolyPolygon(const B2DPolyPolygon& rPolyPolygon);
        B2DPolyPolygon(B2DPolyPolygon&& rPolyPolygon);
        explicit B2DPolyPolygon(const B2DPolygon& rPolygon);
        ~B2DPolyPolygon();

        // assignment operator
        B2DPolyPolygon& operator=(const B2DPolyPolygon& rPolyPolygon);
        B2DPolyPolygon& operator=(B2DPolyPolygon&& rPolyPolygon);

        /// unshare this poly-polygon (and all included polygons) with all internally shared instances
        void makeUnique();

        // compare operators
        bool operator==(const B2DPolyPolygon& rPolyPolygon) const;

        // polygon interface
        sal_uInt32 count() const;

        B2DPolygon const & getB2DPolygon(sal_uInt32 nIndex) const;
        void setB2DPolygon(sal_uInt32 nIndex, const B2DPolygon& rPolygon);

        // test for curve
        bool areControlPointsUsed() const;

        // insert/append single polygon
        void insert(sal_uInt32 nIndex, const B2DPolygon& rPolygon, sal_uInt32 nCount = 1);
        void append(const B2DPolygon& rPolygon, sal_uInt32 nCount = 1);
        void reserve(sal_uInt32 nCount);

        /** Default adaptive subdivision access

            For details refer to B2DPolygon::getDefaultAdaptiveSubdivision()

            @return
            The default subdivision of this polygon
        */
        B2DPolyPolygon getDefaultAdaptiveSubdivision() const;

        /** Get the B2DRange (Rectangle dimensions) of this B2DPolyPolygon

            For details refer to B2DPolygon::getB2DRange()

            @return
            The outer range of the bezier curve/polygon
        */
        B2DRange getB2DRange() const;

        // insert/append multiple polygons
        void insert(sal_uInt32 nIndex, const B2DPolyPolygon& rPolyPolygon);
        void append(const B2DPolyPolygon& rPolyPolygon);

        // remove
        void remove(sal_uInt32 nIndex, sal_uInt32 nCount = 1);

        // reset to empty state
        void clear();

        // closed state
        bool isClosed() const;
        void setClosed(bool bNew);

        // flip polygon direction
        void flip();

        // test if tools::PolyPolygon has double points
        bool hasDoublePoints() const;

        // remove double points, at the begin/end and follow-ups, too
        void removeDoublePoints();

        // apply transformation given in matrix form to the polygon
        void transform(const basegfx::B2DHomMatrix& rMatrix);

        // polygon iterators (same iterator validity conditions as for vector)
        const B2DPolygon* begin() const;
        const B2DPolygon* end() const;
        B2DPolygon* begin();
        B2DPolygon* end();

        // exclusive management op's for SystemDependentData at B2DPolygon
        template<class T>
        std::shared_ptr<T> getSystemDependentData() const
        {
            return std::static_pointer_cast<T>(getSystemDependantDataInternal(typeid(T).hash_code()));
        }

        template<class T, class... Args>
        std::shared_ptr<T> addOrReplaceSystemDependentData(Args&&... args) const
        {
            std::shared_ptr<T> r = std::make_shared<T>(std::forward<Args>(args)...);

            // tdf#129845 only add to buffer if a relevant buffer time is estimated
            if(r->calculateCombinedHoldCyclesInSeconds() > 0)
            {
                basegfx::SystemDependentData_SharedPtr r2(r);
                addOrReplaceSystemDependentDataInternal(r2);
            }

            return r;
        }

    private:
        void addOrReplaceSystemDependentDataInternal(SystemDependentData_SharedPtr& rData) const;
        SystemDependentData_SharedPtr getSystemDependantDataInternal(size_t hash_code) const;
    };

    // typedef for a vector of B2DPolyPolygons
    typedef ::std::vector< B2DPolyPolygon > B2DPolyPolygonVector;

    template< typename charT, typename traits >
    inline std::basic_ostream<charT, traits> & operator <<(
        std::basic_ostream<charT, traits> & stream, const B2DPolyPolygon& poly )
    {
        stream << "[" << poly.count() << ":";
        for (sal_uInt32 i = 0; i < poly.count(); i++)
        {
            if (i > 0)
                stream << ",";
            stream << poly.getB2DPolygon(i);
        }
        stream << "]";

        return stream;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
