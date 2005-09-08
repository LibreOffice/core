/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XBackendEntities.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:16:55 $
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
package ifc.configuration.backend;

import com.sun.star.configuration.backend.XBackendEntities;

import lib.MultiMethodTest;


public class _XBackendEntities extends MultiMethodTest {
    String AdminEntity = "";
    String OwnerEntity = "";
    String adm = "";
    public XBackendEntities oObj;

    public void before() {
        adm = (String) tEnv.getObjRelation("NoAdmin");
    }

    public void _getAdminEntity() {
        if (adm != null) {
            AdminEntity = adm;
            log.println(
                    "This component doesn't have an AdminEntity, setting it to default");
            tRes.tested("getAdminEntity()", true);

            return;
        }

        AdminEntity = oObj.getAdminEntity();

        boolean res = AdminEntity.endsWith("/share/registry");

        if (!res) {
            log.println("Expected the Entity to end with /share/registry");
            log.println("getting: " + AdminEntity + " -- FAILED");
        }

        tRes.tested("getAdminEntity()", res);
    }

    public void _getOwnerEntity() {
        OwnerEntity = oObj.getOwnerEntity();

        boolean res = OwnerEntity.endsWith("/user/registry");

        if (!res) {
            log.println("Expected the Entity to end with /user/registry");
            log.println("getting: " + OwnerEntity + " -- FAILED");
        }

        tRes.tested("getOwnerEntity()", res);
    }

    public void _isEqualEntity() {
        boolean res = true;
        boolean localRes = true;

        try {
            localRes = oObj.isEqualEntity("", "");
            log.println(
                    "No Exception thrown for isEqualEntity(\"\",\"\") -- FAILED");
            res &= false;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Expected Exception (" + e + ") -- OK");
            res &= true;
        }

        try {
            localRes = oObj.isEqualEntity(AdminEntity, AdminEntity);

            if (!localRes) {
                log.println(
                        "isEqualEntity(AdminEntity,AdminEntity) returns false -- FAILED");
            }

            res &= localRes;

            localRes = oObj.isEqualEntity(OwnerEntity, OwnerEntity);

            if (!localRes) {
                log.println(
                        "isEqualEntity(OwnerEntity,OwnerEntity) returns false -- FAILED");
            }

            res &= localRes;

            localRes = !oObj.isEqualEntity(AdminEntity, OwnerEntity);

            if (!localRes) {
                log.println(
                        "isEqualEntity(AdminEntity,OwnerEntity) returns true -- FAILED");
            }

            res &= localRes;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        }

        tRes.tested("isEqualEntity()", res);
    }

    public void _supportsEntity() {
        boolean res = true;
        boolean localRes = true;

        try {
            localRes = oObj.supportsEntity("illegal");
            log.println(
                    "No Exception thrown for supportsEntity(\"\") -- FAILED");
            res &= false;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("Expected Exception (" + e + ") -- OK");
            res &= true;
        }

        try {
            localRes = oObj.supportsEntity(AdminEntity);

            if (adm != null) {
                Object msb = tEnv.getObjRelation("MSB") ;
                if (msb == null){
                    localRes = !localRes;
                    log.println("This Component doesn't support the AdminEntity");
                }
            }

            if (!localRes) {
                log.println(
                        "supportsEntity(AdminEntity) returns false -- FAILED");
            }

            res &= localRes;

            localRes = oObj.supportsEntity(OwnerEntity);

            if (!localRes) {
                log.println(
                        "supportsEntity(OwnerEntity) returns false -- FAILED");
            }

            res &= localRes;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        }

        tRes.tested("supportsEntity()", res);
    }
}