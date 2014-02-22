/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"

#include "ids.hrc"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

bool
getStringRequestArgument(uno::Sequence< uno::Any > const & rArguments,
                         OUString const & rKey,
                         OUString * pValue)
    SAL_THROW(())
{
    for (sal_Int32 i = 0; i < rArguments.getLength(); ++i)
    {
        beans::PropertyValue aProperty;
        if ((rArguments[i] >>= aProperty) && aProperty.Name == rKey)
        {
            OUString aValue;
            if (aProperty.Value >>= aValue)
            {
                if (pValue)
                    *pValue = aValue;
                return true;
            }
        }
    }
    return false;
}

bool
getBoolRequestArgument(uno::Sequence< uno::Any > const & rArguments,
                       OUString const & rKey,
                       bool * pValue)
    SAL_THROW(())
{
    for (sal_Int32 i = 0; i < rArguments.getLength(); ++i)
    {
        beans::PropertyValue aProperty;
        if ((rArguments[i] >>= aProperty) && aProperty.Name == rKey)
        {
            sal_Bool bValue = sal_Bool();
            if (aProperty.Value >>= bValue)
            {
                if (pValue)
                    *pValue = bValue;
                return true;
            }
        }
    }
    return false;
}

bool
getResourceNameRequestArgument(uno::Sequence< uno::Any > const & rArguments,
                               OUString * pValue)
    SAL_THROW(())
{
    if (!getStringRequestArgument(rArguments, "Uri",  pValue))
        return false;
    
    
    if (pValue
        && pValue->matchIgnoreAsciiCase("file:"))
        getStringRequestArgument(rArguments, "ResourceName", pValue);
    return true;
}

} 

bool
UUIInteractionHelper::handleInteractiveIOException(
        uno::Reference< task::XInteractionRequest > const & rRequest,
        bool bObtainErrorStringOnly,
        bool & bHasErrorString,
        OUString & rErrorString)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());
    bHasErrorString = false;

    ucb::InteractiveIOException aIoException;
    if (aAnyRequest >>= aIoException)
    {
        uno::Sequence< uno::Any > aRequestArguments;
        ucb::InteractiveAugmentedIOException aAugmentedIoException;
        if (aAnyRequest >>= aAugmentedIoException)
            aRequestArguments = aAugmentedIoException.Arguments;

        ErrCode nErrorCode;
        std::vector< OUString > aArguments;
        static ErrCode const
            aErrorCode[ucb::IOErrorCode_WRONG_VERSION + 1][2]
            = { { ERRCODE_IO_ABORT, ERRCODE_UUI_IO_ABORT }, 
                { ERRCODE_IO_ACCESSDENIED, ERRCODE_UUI_IO_ACCESSDENIED },
                
                { ERRCODE_IO_ALREADYEXISTS,
                  ERRCODE_UUI_IO_ALREADYEXISTS }, 
                { ERRCODE_IO_BADCRC, ERRCODE_UUI_IO_BADCRC }, 
                { ERRCODE_IO_CANTCREATE, ERRCODE_UUI_IO_CANTCREATE },
                
                { ERRCODE_IO_CANTREAD, ERRCODE_UUI_IO_CANTREAD },
                
                { ERRCODE_IO_CANTSEEK, ERRCODE_UUI_IO_CANTSEEK },
                
                { ERRCODE_IO_CANTTELL, ERRCODE_UUI_IO_CANTTELL },
                
                { ERRCODE_IO_CANTWRITE, ERRCODE_UUI_IO_CANTWRITE },
                
                { ERRCODE_IO_CURRENTDIR, ERRCODE_UUI_IO_CURRENTDIR },
                
                { ERRCODE_IO_DEVICENOTREADY, ERRCODE_UUI_IO_NOTREADY },
                
                { ERRCODE_IO_NOTSAMEDEVICE,
                  ERRCODE_UUI_IO_NOTSAMEDEVICE }, 
                { ERRCODE_IO_GENERAL, ERRCODE_UUI_IO_GENERAL }, 
                { ERRCODE_IO_INVALIDACCESS,
                  ERRCODE_UUI_IO_INVALIDACCESS }, 
                { ERRCODE_IO_INVALIDCHAR, ERRCODE_UUI_IO_INVALIDCHAR },
                
                { ERRCODE_IO_INVALIDDEVICE,
                  ERRCODE_UUI_IO_INVALIDDEVICE }, 
                { ERRCODE_IO_INVALIDLENGTH,
                  ERRCODE_UUI_IO_INVALIDLENGTH }, 
                { ERRCODE_IO_INVALIDPARAMETER,
                  ERRCODE_UUI_IO_INVALIDPARAMETER }, 
                { ERRCODE_IO_ISWILDCARD, ERRCODE_UUI_IO_ISWILDCARD },
                
                { ERRCODE_IO_LOCKVIOLATION,
                  ERRCODE_UUI_IO_LOCKVIOLATION }, 
                { ERRCODE_IO_MISPLACEDCHAR,
                  ERRCODE_UUI_IO_MISPLACEDCHAR }, 
                { ERRCODE_IO_NAMETOOLONG, ERRCODE_UUI_IO_NAMETOOLONG },
                
                { ERRCODE_IO_NOTEXISTS, ERRCODE_UUI_IO_NOTEXISTS },
                
                { ERRCODE_IO_NOTEXISTSPATH,
                  ERRCODE_UUI_IO_NOTEXISTSPATH }, 
                { ERRCODE_IO_NOTSUPPORTED, ERRCODE_UUI_IO_NOTSUPPORTED },
                
                { ERRCODE_IO_NOTADIRECTORY,
                  ERRCODE_UUI_IO_NOTADIRECTORY }, 
                { ERRCODE_IO_NOTAFILE, ERRCODE_UUI_IO_NOTAFILE },
                
                { ERRCODE_IO_OUTOFSPACE, ERRCODE_UUI_IO_OUTOFSPACE },
                
                { ERRCODE_IO_TOOMANYOPENFILES,
                  ERRCODE_UUI_IO_TOOMANYOPENFILES },
                
                { ERRCODE_IO_OUTOFMEMORY, ERRCODE_UUI_IO_OUTOFMEMORY },
                
                { ERRCODE_IO_PENDING, ERRCODE_UUI_IO_PENDING }, 
                { ERRCODE_IO_RECURSIVE, ERRCODE_UUI_IO_RECURSIVE },
                
                { ERRCODE_IO_UNKNOWN, ERRCODE_UUI_IO_UNKNOWN }, 
                { ERRCODE_IO_WRITEPROTECTED,
                  ERRCODE_UUI_IO_WRITEPROTECTED }, 
                { ERRCODE_IO_WRONGFORMAT, ERRCODE_UUI_IO_WRONGFORMAT },
                
                { ERRCODE_IO_WRONGVERSION,
                  ERRCODE_UUI_IO_WRONGVERSION } }; 
        switch (aIoException.Code)
        {
        case ucb::IOErrorCode_CANT_CREATE:
            {
                OUString aArgFolder;
                if (getStringRequestArgument(aRequestArguments, "Folder", &aArgFolder))
                {
                    OUString aArgUri;
                    if (getResourceNameRequestArgument(aRequestArguments,
                                                       &aArgUri))
                    {
                        nErrorCode = ERRCODE_UUI_IO_CANTCREATE;
                        aArguments.reserve(2);
                        aArguments.push_back(aArgUri);
                        aArguments.push_back(aArgFolder);
                    }
                    else
                    {
                        nErrorCode = ERRCODE_UUI_IO_CANTCREATE_NONAME;
                        aArguments.push_back(aArgFolder);
                    }
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }

        case ucb::IOErrorCode_DEVICE_NOT_READY:
            {
                OUString aArgUri;
                if (getResourceNameRequestArgument(aRequestArguments,
                                                   &aArgUri))
                {
                    OUString aResourceType;
                    getStringRequestArgument(aRequestArguments, "ResourceType", &aResourceType);
                    bool bRemovable = false;
                    getBoolRequestArgument(aRequestArguments, "Removable", &bRemovable);
                    nErrorCode = aResourceType == "volume"
                        ? (bRemovable
                           ? ERRCODE_UUI_IO_NOTREADY_VOLUME_REMOVABLE
                           : ERRCODE_UUI_IO_NOTREADY_VOLUME)
                        : (bRemovable
                           ? ERRCODE_UUI_IO_NOTREADY_REMOVABLE
                           : ERRCODE_UUI_IO_NOTREADY);
                    aArguments.push_back(aArgUri);
                }
                else
            nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }

        case ucb::IOErrorCode_DIFFERENT_DEVICES:
            {
                OUString aArgVolume;
                OUString aArgOtherVolume;
                if (getStringRequestArgument(aRequestArguments, "Volume", &aArgVolume)
                    && getStringRequestArgument(aRequestArguments, "OtherVolume",
                        &aArgOtherVolume))
                {
                    nErrorCode = aErrorCode[aIoException.Code][1];
                    aArguments.reserve(2);
                    aArguments.push_back(aArgVolume);
                    aArguments.push_back(aArgOtherVolume);
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
        }

        case ucb::IOErrorCode_NOT_EXISTING:
            {
                OUString aArgUri;
                if (getResourceNameRequestArgument(aRequestArguments,
                           &aArgUri))
                {
                    OUString aResourceType;
                    getStringRequestArgument(aRequestArguments, "ResourceType",
                                            &aResourceType);
                    nErrorCode = aResourceType == "volume"
                        ? ERRCODE_UUI_IO_NOTEXISTS_VOLUME
                        : (aResourceType == "folder"
                           ? ERRCODE_UUI_IO_NOTEXISTS_FOLDER
                           : ERRCODE_UUI_IO_NOTEXISTS);
                    aArguments.push_back(aArgUri);
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }

        default:
            {
                OUString aArgUri;
                if (getResourceNameRequestArgument(aRequestArguments,
                                                   &aArgUri))
                {
                    nErrorCode = aErrorCode[aIoException.Code][1];
                    aArguments.push_back(aArgUri);
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }
        }

        handleErrorHandlerRequest(aIoException.Classification,
                                  nErrorCode,
                                  aArguments,
                                  rRequest->getContinuations(),
                                  bObtainErrorStringOnly,
                                  bHasErrorString,
                                  rErrorString);
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
