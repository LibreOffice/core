/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

namespace com.sun.star.uno
{
    public class Exception : System.Exception
    {
        public new string Message { get; set; }
        public IQueryInterface Context { get; set; }
        
        public Exception() { }
        public Exception(string Message, IQueryInterface Context)
        {
            this.Message = Message;
            this.Context = Context;
        }
    }
}
