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



#ifndef WRAP_POLYGON_HANDLER_HXX
#define WRAP_POLYGON_HANDLER_HXX

#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <resourcemodel/LoggedResources.hxx>
#include <resourcemodel/Fraction.hxx>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using resourcemodel::Fraction;

class WrapPolygon
{
public:
    typedef ::std::deque<awt::Point> Points_t;
    typedef ::boost::shared_ptr<WrapPolygon> Pointer_t;

private:
    Points_t mPoints;

public:
    WrapPolygon();
    virtual ~WrapPolygon();

    void addPoint(const awt::Point & rPoint);

    Points_t::const_iterator begin() const;
    Points_t::const_iterator end() const;
    Points_t::iterator begin();
    Points_t::iterator end();

    size_t size() const;

    WrapPolygon::Pointer_t move(const awt::Point & rMove);
    WrapPolygon::Pointer_t scale(const Fraction & rFractionX, const Fraction & rFractionY);
    WrapPolygon::Pointer_t correctWordWrapPolygon(const awt::Size & rSrcSize, const awt::Size & rDstSize);
    drawing::PointSequenceSequence getPointSequenceSequence() const;
};

class WrapPolygonHandler : public LoggedProperties
{
public:
    WrapPolygonHandler();
    virtual ~WrapPolygonHandler();

    WrapPolygon::Pointer_t getPolygon();

private:
    WrapPolygon::Pointer_t mpPolygon;

    sal_uInt32 mnX;
    sal_uInt32 mnY;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

};

}}

#endif // WRAP_POLYGON_HANDLER_HXX
