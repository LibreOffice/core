//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//


// Global variable to reference LibreOffice functions
var LOinterface = LOkit.init()



class LOkit
{
    init()
    {
        LOkit_Init()
    }



    func Initialize()
    {
        let myInput = "ping command jan var her"
        
        LOkit_ClientCommand(myInput)

        let my2Input = "mouse command jan var her"
        
        LOkit_ClientCommand(my2Input)
}
    
    func DeleteCurrentDocument()
    {
        
    }
}

