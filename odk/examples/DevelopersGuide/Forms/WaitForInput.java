/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
class WaitForInput extends java.lang.Thread
{
    private Object      m_aToNotify;
    private boolean     m_bDone;

    public WaitForInput( Object aToNotify )
    {
        m_aToNotify = aToNotify;
        m_bDone = false;
    }

    public boolean isDone()
    {
        return m_bDone;
    }

    @Override
    public void run()
    {
        try
        {
            System.out.println( "\npress 'enter' to exit demo" );
            System.in.read();

            m_bDone = true;
            // notify that the user pressed the key
            synchronized (m_aToNotify)
            {
                m_aToNotify.notify();
            }
        }
        catch( java.lang.Exception e )
        {
            // not really interested in
            System.err.println( e );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
