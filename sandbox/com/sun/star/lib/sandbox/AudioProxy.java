/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AudioProxy.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:03:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.lib.sandbox;

import java.applet.AudioClip;

import java.util.Vector;

class AudioProxy implements AudioClip, Holder {
    private static final boolean DEBUG = false;
    private static int instances;

    private AudioClip audioClip;
    private Vector cargo = new Vector();

    AudioProxy(AudioClip audioClip) {
        this.audioClip = audioClip;
        instances ++;
    }

    public void play() {
        audioClip.play();
    }

    public void loop() {
        audioClip.loop();
    }

    public void stop() {
        audioClip.stop();
    }

    public void addCargo(Object object) {
        cargo.addElement(object);
    }

    public void removeCargo(Object object) {
        cargo.removeElement(object);
    }

    public void finalize() {
        if(DEBUG) System.err.println("#### AudioProxy.finalize");
        instances --;
    }
}

