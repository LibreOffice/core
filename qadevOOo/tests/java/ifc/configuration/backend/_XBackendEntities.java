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
package ifc.configuration.backend;

import com.sun.star.configuration.backend.XBackendEntities;

import lib.MultiMethodTest;


public class _XBackendEntities extends MultiMethodTest {
    String AdminEntity = "";
    String OwnerEntity = "";
    String adm = "";
    public XBackendEntities oObj;

    @Override
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