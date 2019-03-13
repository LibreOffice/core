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
#ifndef INCLUDED_CHART2_SOURCE_INC_MEDIADESCRIPTORHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_MEDIADESCRIPTORHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "charttoolsdllapi.hxx"

namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace embed { class XStorage; } } } }
namespace com { namespace sun { namespace star { namespace io { class XInputStream; } } } }
namespace com { namespace sun { namespace star { namespace io { class XOutputStream; } } } }
namespace com { namespace sun { namespace star { namespace io { class XStream; } } } }

/*
* This class helps to read and write the properties mentioned in the service description
* com.sun.star.document.MediaDescriptor from and to a sequence of PropertyValues.
* Properties that are not mentioned in the service description
* are stored in the member AdditionalProperties.
*
* As an additional feature this helper class can generate a reduced sequence of PropertyValues
* that does not contain properties which are known to be only view relevant. This
* reduced sequence than might be attached to a model directly.
*/

namespace apphelper
{

class OOO_DLLPUBLIC_CHARTTOOLS MediaDescriptorHelper final
{
public:
    MediaDescriptorHelper( const css::uno::Sequence< css::beans::PropertyValue > & rMediaDescriptor );

    const css::uno::Sequence< css::beans::PropertyValue >& getReducedForModel() { return m_aModelProperties;}

    //all properties given in the constructor are stored in the following three sequences

    //properties which should be given to a model are additionally stored in this sequence (not documented properties and deprecated properties are not included!)
    css::uno::Sequence< css::beans::PropertyValue >
                        m_aModelProperties; //these are properties which are not described in service com.sun.star.document.MediaDescriptor

    OUString            FilterName; //internal filter name.
    bool                ISSET_FilterName;

    OUString            HierarchicalDocumentName;

    css::uno::Reference< css::io::XOutputStream >
                        OutputStream; //a stream to receive the document data for saving
    bool                ISSET_OutputStream;
    css::uno::Reference< css::io::XInputStream >
                        InputStream; //content of document.
    bool                ISSET_InputStream;

    bool                ReadOnly; //open document readonly.

    OUString            URL;// FileName, URL of the document.
    bool                ISSET_URL;

    // new framework objects
    css::uno::Reference< css::embed::XStorage >
                        Storage;
    bool                ISSET_Storage;
    css::uno::Reference< css::io::XStream >
                        Stream;
    bool                ISSET_Stream;

private:
    SAL_DLLPRIVATE void impl_init();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
