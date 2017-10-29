//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit



// DocumentActions is the main menu popover started from DocumentController
// it allows the user to invoke global actions and delegates the processing
// back to the DocumentController
class DocumentActions: UITableViewController
{
    // Pointer to callback class (in reality instance of DocumentController)
    var delegate  : MenuDelegate?



    // set by DocumentController before invoking the menu
    // the variable is used to control which menu entries are active
    var isDocActive : Bool = false



    // Reference to instances of the buttons
    @IBOutlet weak var buttonOpen: UIButton!
    @IBOutlet weak var buttonProperties: UIButton!
    @IBOutlet weak var buttonSave: UIButton!
    @IBOutlet weak var buttonClose: UIButton!
    @IBOutlet weak var buttonSaveAs: UIButton!
    @IBOutlet weak var buttonSaveAsPDF: UIButton!
    @IBOutlet weak var buttonPrint: UIButton!



    // called once controller is loaded
    override func viewDidLoad()
    {
        super.viewDidLoad()

        // Some menu entries are only active when a document is loaded
        buttonSave.isEnabled = isDocActive
        buttonProperties.isEnabled = isDocActive
        buttonClose.isEnabled = isDocActive
        buttonSaveAs.isEnabled = isDocActive
        buttonSaveAsPDF.isEnabled = isDocActive
        buttonPrint.isEnabled = isDocActive
    }



    // Called when user click on a menu entry (all entries goes here)
    // see storyboard Document actions scene for details
    @IBAction func actionMenuSelect(_ sender: UIButton)
    {
        // make popover go away, when this function returns
        dismiss(animated: false)

        // inform DocumentController about the selected entry
        delegate?.actionMenuSelected(sender.tag)
    }
}



// Protocol for action popover callback
protocol MenuDelegate
{
    // inform delegate about selected menu entry
    func actionMenuSelected(_ tag : Int)
}

