/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.test.vcl.client;


/**
 * ID of GUI controls may have two types: number or string.
 * @deprecated From AOO3.4, all IDs should be string
 */
public class SmartId {

    private long id = 0;

    private String sid = null;

    public SmartId(long id) {
        super();
        this.id = id;
    }

    public SmartId(String sid) {
        super();
        this.sid = sid;
    }

    public long getId() {
        return id;
    }

    public String getSid() {
        return sid;
    }

    public String toString() {
        if (sid == null)
            return Long.toString(id);
        else
            return sid;
    }

    public int hashCode() {
        if (sid == null)
            return new Long(id).hashCode();
        return sid.hashCode();
    }

    public boolean equals (Object o) {
        if (!(o instanceof SmartId))
            return false;
        SmartId id2 = (SmartId) o;
        return id2.id == this.id && ((this.sid == null && id2.sid == null) || (this.sid != null && this.sid.equals(id2.sid)));
    }
}
