/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_TRACKCHANGESHANDLER_HXX
#define INCLUDED_TRACKCHANGESHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <DomainMapper_Impl.hxx>

namespace writerfilter {
namespace dmapper
{
/** Handler for sprms that contain 'track changes' attributes
    - Author
    - Date
    - ID
    (This class is based on work done in 'MeasureHandler')
 */
class TrackChangesHandler : public LoggedProperties
{
    RedlineParamsPtr     m_pRedlineParams;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    TrackChangesHandler( sal_Int32 nToken );
    virtual ~TrackChangesHandler();

    // Compute the UNO properties for the track changes object based on the received tokens.
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> getRedlineProperties() const;
};
typedef boost::shared_ptr
    < TrackChangesHandler >  TrackChangesHandlerPtr;
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
