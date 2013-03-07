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
package org.libreoffice.report;

/**
 * Expressions are simple computation components.
 *
 * Expressions are always assumed to be immutable. They are not allowed to
 * change their state and it is not guaranteed, in which order they get called.
 * If the expression has been called before, the last computed value will be
 * available from the datarow.
 *
 * This construct allows us to write expressions in the form
 * "Sum := Sum + Column".
 *
 * Multiple calls to getValue on the same expression instance must return the
 * same value (assuming that the datarow passed in is the same).
 */
public interface ReportExpression
{

    void setParameters(Object[] parameters);

    Object getParameters();

    Object getValue(DataRow row);
}
