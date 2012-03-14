/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _CHART2_DUMPHELPER_HXX
#define _CHART2_DUMPHELPER_HXX

#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/DoubleSequenceSequence.hpp>

#define ENABLE_DUMP 0

namespace chart
{

class DumpHelper
{
private:
    //XmlTextWriterPtr pWriter;

public:
   //void writeStripe(const Stripe& rStripe);

   void writeElement(const char*);
   void writeAttribute(const char* pAttrName, const char* pAttrValue);
   void writeAttribute(const char* pAttrName, const com::sun::star::drawing::Position3D& rPosition);
   void writeAttribute(const char* pAttrName, const rtl::OUString& rName);
   void writeAttribute(const char* pAttrName, const sal_Int32);
   void writeAttribute(const char* pAttrName, const com::sun::star::drawing::Direction3D& rPosition);
   void writeAttribute(const char* pAttrName, const com::sun::star::drawing::PointSequenceSequence& rPoints);
   void writeAttribute(const char* pAttrName, const com::sun::star::drawing::PolyPolygonShape3D& rPoints);
   void writeAttribute(const char* pAttrName, const com::sun::star::drawing::PolyPolygonBezierCoords& rCoords);
   void writePointElement(const com::sun::star::awt::Point& rPoint);
   void writeDoubleSequence(const char* pName, const com::sun::star::drawing::DoubleSequenceSequence& rSequence);
   void endElement();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
