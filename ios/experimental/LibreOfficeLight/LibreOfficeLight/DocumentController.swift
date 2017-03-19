//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit



class DocumentController: UIViewController, DocumentActionsControlDelegate
{
    // Show sidemenu (part of documentcontroller)
    @IBAction func doMenu(_ sender: UIBarButtonItem)
    {
        if (sender.tag == 10) {
            sender.tag = 0;

            let viewMenuBack : UIView = view.subviews.last!

            UIView.animate(withDuration: 0.3, animations: { () -> Void in
                var frameMenu : CGRect = viewMenuBack.frame
                frameMenu.origin.x = -1 * UIScreen.main.bounds.size.width
                viewMenuBack.frame = frameMenu
                viewMenuBack.layoutIfNeeded()
                viewMenuBack.backgroundColor = UIColor.clear
                }, completion: { (finished) -> Void in
                    viewMenuBack.removeFromSuperview()
                })
            return
        }

        sender.isEnabled = false
        sender.tag = 10

        let sidebar : SidebarController = self.storyboard!.instantiateViewController(withIdentifier: "SidebarController") as! SidebarController
        view.addSubview(sidebar.view)
        addChildViewController(sidebar)
        sidebar.view.layoutIfNeeded()


        sidebar.view.frame=CGRect(x: 0 - UIScreen.main.bounds.size.width, y: 0, width: UIScreen.main.bounds.size.width, height: UIScreen.main.bounds.size.height);

        UIView.animate(withDuration: 0.3, animations: { () -> Void in
            sidebar.view.frame=CGRect(x: 0, y: 0, width: UIScreen.main.bounds.size.width, height: UIScreen.main.bounds.size.height);
            sender.isEnabled = true
            }, completion:nil)
    }



    // Last stop before displaying popover
    override func prepare(for segue: UIStoryboardSegue, sender: Any?)
    {
        if segue.identifier == "showActions" {
            let vc = segue.destination as! DocumentActions
            vc.delegate = self

            // JIX, TO BE CHANGED
            vc.isDocActive = true
        }
    }



    func actionNew(_ name : String)
    {
        // JIX Close active documents if any
        // Start new (with default name

        // Only interact with DocumentBrowser

    }



    func actionOpen()
    {
        // JIX Close active documents if any
        // Present FileManager
        performSegue(withIdentifier: "showFileManager", sender: self)

        // start DocumentBrowser with new document
    }



    // Called when returning from filemanager
    @IBAction func returned(segue: UIStoryboardSegue)
    {
        // JIX actually open document
        print("I returned")
    }




    func actionDelete()
    {
        // JIX Close active documents if any
        // Delete document
    }



    func actionSave()
    {
        // call save in DocumentBrowser

    }



    func actionSaveAs(_ name : String)
    {
        // call saveas in DocumentBrowser

    }



    func actionPDF()
    {
        // call savePDF in documentBrowser
    }



    func actionPrint()
    {
        // call print in DocumentBrowser
    }



    override func viewDidLoad()
    {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
    }



    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
}



// Protocol for action popover callback
protocol DocumentActionsControlDelegate
{
    func actionNew(_ name : String)
    func actionOpen()
    func actionDelete()
    func actionSave()
    func actionSaveAs(_ name : String)
    func actionPDF()
    func actionPrint()
}



class DocumentActions: UITableViewController
{
    // Pointer to callback class
    var delegate  : DocumentActionsControlDelegate?
    var isDocActive : Bool = false

    // Calling class might enable/disable each button
    @IBOutlet weak var buttonNew: UIButton!
    @IBOutlet weak var buttonOpen: UIButton!
    @IBOutlet weak var buttonDelete: UIButton!
    @IBOutlet weak var buttonSave: UIButton!
    @IBOutlet weak var buttonSaveAs: UIButton!
    @IBOutlet weak var buttonPDF: UIButton!
    @IBOutlet weak var buttonPrint: UIButton!


    // Actions
    @IBAction func doOpen(_ sender: UIButton)
    {
        delegate?.actionOpen()
        dismiss(animated: false)
    }



    @IBAction func doDelete(_ sender: UIButton)
    {
        delegate?.actionDelete()
        dismiss(animated: false)
    }



    @IBAction func doSave(_ sender: UIButton)
    {
        delegate?.actionSave()
        dismiss(animated: false)
    }



    @IBAction func doPDF(_ sender: UIButton)
    {
        delegate?.actionPDF()
        dismiss(animated: false)
    }



    @IBAction func doPrint(_ sender: UIButton)
    {
        delegate?.actionPrint()
        dismiss(animated: false)
    }



    override func viewDidLoad()
    {
        super.viewDidLoad()
        buttonDelete.isEnabled = isDocActive
        buttonSave.isEnabled = isDocActive
        buttonSaveAs.isEnabled = isDocActive
        buttonPDF.isEnabled = isDocActive
        buttonPrint.isEnabled = isDocActive
    }



    // Last stop before displaying popover
    override func prepare(for segue: UIStoryboardSegue, sender: Any?)
    {
        let vc = segue.destination as! setNameAction
        vc.delegateDoc = self.delegate
        vc.protocolActionToPerform = (segue.identifier == "showNew") ? 2 : 3
    }
}

