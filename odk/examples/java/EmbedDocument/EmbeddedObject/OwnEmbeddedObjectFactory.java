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

import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;

public final class OwnEmbeddedObjectFactory extends WeakBase
   implements com.sun.star.lang.XServiceInfo,
              com.sun.star.embed.XEmbedObjectFactory
{
    private final XComponentContext m_xContext;
    private static final String m_implementationName = OwnEmbeddedObjectFactory.class.getName();
    private static final String[] m_serviceNames = {
        "org.openoffice.examples.embedding.Factory69474366FD6F480683748EDD1B6E771D" };
    private static final byte[] m_classID = { 0x69, 0x47, 0x43, 0x66, (byte)0xFD, 0x6F, 0x48, 0x06, (byte)0x83, 0x74, (byte)0x8E, (byte)0xDD, 0x1B, 0x6E, 0x77, 0x1D };


    public OwnEmbeddedObjectFactory( XComponentContext context )
    {
        m_xContext = context;
    };

    public static XSingleComponentFactory __getComponentFactory( String sImplementationName ) {
        XSingleComponentFactory xFactory = null;

        if ( sImplementationName.equals( m_implementationName ) )
            xFactory = Factory.createComponentFactory(OwnEmbeddedObjectFactory.class, m_serviceNames);
        return xFactory;
    }

    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.services.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo( XRegistryKey xRegistryKey ) {
//         return Factory.writeRegistryServiceInfo(m_implementationName,
//                                                 m_serviceNames,
//                                                 xRegistryKey);
//     }

    // com.sun.star.lang.XServiceInfo:
    public String getImplementationName() {
         return m_implementationName;
    }

    public boolean supportsService( String sService ) {
        int len = m_serviceNames.length;

        for( int i=0; i < len; i++) {
            if (sService.equals(m_serviceNames[i]))
                return true;
        }
        return false;
    }

    public String[] getSupportedServiceNames() {
        return m_serviceNames;
    }

    // com.sun.star.embed.XEmbedObjectFactory:
    public Object createInstanceUserInit(byte[] aClassID, String sClassName, com.sun.star.embed.XStorage xStorage, String sEntName, int nEntryConnectionMode, com.sun.star.beans.PropertyValue[] aArgs, com.sun.star.beans.PropertyValue[] aObjectArgs) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.io.IOException, com.sun.star.uno.Exception
    {
        if ( xStorage == null || sEntName == null || sEntName.length() == 0 )
            throw new com.sun.star.lang.IllegalArgumentException();

        if ( nEntryConnectionMode == com.sun.star.embed.EntryInitModes.DEFAULT_INIT )
        {
            if ( aClassID != null && aClassID.length != 0 )
            {
                if ( aClassID.length != m_classID.length )
                    throw new com.sun.star.lang.IllegalArgumentException();

                for ( int i = 0; i < aClassID.length; i++ )
                    if ( aClassID[i] != m_classID[i] )
                        throw new com.sun.star.lang.IllegalArgumentException();
            }
            else if ( !xStorage.hasByName( sEntName ) )
                throw new com.sun.star.lang.IllegalArgumentException();
        }
        else if ( nEntryConnectionMode == com.sun.star.embed.EntryInitModes.TRUNCATE_INIT )
        {
            if ( aClassID.length != m_classID.length )
                throw new com.sun.star.lang.IllegalArgumentException();

            for ( int i = 0; i < m_classID.length; i++ )
                if ( aClassID[i] != m_classID[i] )
                    throw new com.sun.star.lang.IllegalArgumentException();
        }

        OwnEmbeddedObject aObject = new OwnEmbeddedObject( m_xContext, m_classID );
        aObject.setPersistentEntry( xStorage, sEntName, nEntryConnectionMode, aArgs, aObjectArgs );

        return aObject;
    }

}
