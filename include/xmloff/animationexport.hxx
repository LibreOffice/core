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

#ifndef INCLUDED_XMLOFF_ANIMATIONEXPORT_HXX
#define INCLUDED_XMLOFF_ANIMATIONEXPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>

#include <salhelper/simplereferenceobject.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace animations { class XAnimationNode; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }

class SvXMLExport;

namespace xmloff
{
class AnimationsExporterImpl;

class XMLOFF_DLLPUBLIC AnimationsExporter : public salhelper::SimpleReferenceObject
{
    std::unique_ptr<AnimationsExporterImpl>  mpImpl;

public:
    AnimationsExporter( SvXMLExport& rExport, const css::uno::Reference< css::beans::XPropertySet >& xPageProps  );
    virtual ~AnimationsExporter() override;

    void prepare( const css::uno::Reference< css::animations::XAnimationNode >& xRootNode );
    void exportAnimations( const css::uno::Reference< css::animations::XAnimationNode >& xRootNode );
};

}

#endif // INCLUDED_XMLOFF_ANIMATIONEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
