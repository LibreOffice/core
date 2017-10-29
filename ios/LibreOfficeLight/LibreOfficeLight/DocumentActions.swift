//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit



// Protocol for action popover callback
protocol MenuDelegate
{
    func actionMenuSelected(_ tag : Int)
}



class DocumentActions: UITableViewController
{
    // Pointer to callback class
    var delegate  : MenuDelegate?
    var isDocActive : Bool = false

    // Calling class might enable/disable each button
    @IBAction func actionMenuSelect(_ sender: UIButton)
    {
        dismiss(animated: false)
        delegate?.actionMenuSelected(sender.tag)
    }

    @IBOutlet weak var buttonClose: UIButton!
    @IBOutlet weak var buttonProperties: UIButton!
    @IBOutlet weak var buttonNew: UIButton!
    @IBOutlet weak var buttonOpen: UIButton!
    @IBOutlet weak var buttonSave: UIButton!
    @IBOutlet weak var buttonSaveAs: UIButton!
    @IBOutlet weak var buttonSaveAsPDF: UIButton!
    @IBOutlet weak var buttonPrint: UIButton!
    @IBOutlet weak var buttonCopy: UIButton!
    @IBOutlet weak var buttonMove: UIButton!
    @IBOutlet weak var buttonDelete: UIButton!

    override func viewDidLoad()
    {
        super.viewDidLoad()
        buttonSave.isEnabled = isDocActive
        buttonSaveAs.isEnabled = isDocActive
        buttonSaveAsPDF.isEnabled = isDocActive
        buttonPrint.isEnabled = isDocActive
    }
}
