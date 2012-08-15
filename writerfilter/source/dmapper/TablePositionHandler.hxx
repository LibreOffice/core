/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_POSITIONHANDLER_HXX
#define INCLUDED_POSITIONHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

namespace writerfilter {
    namespace dmapper {

        /// Handler for floating table positioning
        class WRITERFILTER_DLLPRIVATE TablePositionHandler
            : public LoggedProperties
        {
            OUString m_aVertAnchor;

            // Properties
            virtual void lcl_attribute(Id Name, Value & val);
            virtual void lcl_sprm(Sprm & sprm);

            public:
            TablePositionHandler();
            virtual ~TablePositionHandler();

            OUString getVertAnchor() const;
        };

        typedef boost::shared_ptr<TablePositionHandler> TablePositionHandlerPtr;
    } // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
