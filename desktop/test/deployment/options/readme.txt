Important: The hander component extensionoptions.jar in the extensions may not 
contain exactly the same sources as the one build in the handler directory. To
make sure that debugging works build the handler directory and put the 
extensionoptions.jar into the extension.



leaf1.oxt: Defines a leaf under the node WriterNode
================================================================================

leaf1mod.oxt: Defines a leaf under the node WriterNode

It has a douplicate entry in the manifest.xml (OptionsDialog.xcu). This would cause a DisposedException when uninstalling on OOo 3.0 and prevent the extension from being uninstalled. This is actually a bug of the extensions. However, the error is difficult to investigate. Therefore this was fixed to make OOo more robust (i96690).
================================================================================

leaf2.oxt: Defines a leaf under a node that has a name which requires special 
"xml encoding". The name is "My Writer's & Settings". The node is not assigned 
to a Module and the Node/AllModules property is not true. This is a typical 
scenario when a Node had been added to an existing Module and later the Module 
was removed. This is a situation which actually should not occur. In this case 
DO NOT show the Node in the OOo's options dialog, because it shows only nodes 
for a particular module and in this case the Module for the Node is unknown.
In the Extension Manager's 
options dialog this Node can be shown because the Module is irrelevant.
See also nodes5.oxt.
================================================================================

leaves1.oxt: multiple ordered leaves under available nodes. The leaves Lables are 
localized for en-US and de. The following leaves should appear:

Writer:
-leaves1 Writer 1 en-US
-leaves1 Writer 2 en-US
-leaves1 Writer 3 en-US

Calc:
-leaves1 Calc 3 en-US
-leaves1 Calc 3 en-US
-leaves1 Calc 3 en-US

Draw:
-leaves1 Draw 3 en-US
-leaves1 Draw 3 en-US
-leaves1 Draw 3 en-US

If a german office is used then the strings contain "de" instead of "en-US".
================================================================================

leaves2.oxt: Same as leaves1.oxt. Use together with leaves1.oxt to test the 
grouping of leaves.
================================================================================

leaves3.oxt: Same as leaves1.oxt, but the leaves are not ordered.
================================================================================

nodes1.oxt: Defines one node which has AllModules set and which has 
no children. Therefore this node should not be displayed.
================================================================================

nodes2.oxt: Defines 3 nodes which use AllModules and which form an 
ordered group. Every node defines also 3 nodes which have a determined order.

-nodes2 node 1 en-US
     -nodes2 node 1 leaf 1 en-US
     -nodes2 node 1 leaf 2 en-US
     -nodes2 node 1 leaf 3 en-US

-nodes2 node 2 en-US
     -nodes2 node 2 leaf 1 en-US
     -nodes2 node 2 leaf 2 en-US
     -nodes2 node 2 leaf 3 en-US

-nodes2 node 3 en-US
     -nodes2 node 3 leaf 1 en-US
     -nodes2 node 3 leaf 2 en-US
     -nodes2 node 3 leaf 3 en-US

================================================================================

nodes3.oxt: Defines 3 nodes which are placed under different existing Modules. 
The nodes and there leaves are ordered.

Context Writer:
- nodes3 node 1
        nodes3 node 1 leaf 1 en-US
	nodes3 node 1 leaf 2 en-US
	nodes3 node 1 leaf 3 en-US

- nodes3 node 2
        nodes3 node 2 leaf 1 en-US
	nodes3 node 2 leaf 2 en-US
	nodes3 node 2 leaf 3 en-US

- nodes3 node 3
        nodes3 node 3 leaf 1 en-US
	nodes3 node 3 leaf 2 en-US
	nodes3 node 3 leaf 3 en-US

Context Calc:
- nodes3 node 1
        nodes3 node 1 leaf 1 en-US
	nodes3 node 1 leaf 2 en-US
	nodes3 node 1 leaf 3 en-US

- nodes3 node 3
        nodes3 node 3 leaf 1 en-US
	nodes3 node 3 leaf 2 en-US
	nodes3 node 3 leaf 3 en-US

Context Draw:
- nodes3 node 2
        nodes3 node 2 leaf 1 en-US
	nodes3 node 2 leaf 2 en-US
	nodes3 node 2 leaf 3 en-US

================================================================================

nodes4.oxt: Same as nodes3.oxt. Use together with nodes3.txt to test the 
grouping of nodes.
================================================================================

nodes5.oxt: Defines a node which in turn defines 3 leaves. The Node
is not assigned to a Module and the AllModule property is false (which is the 
default).This may happen when a node 
had been added to an already existing Module and then this Module was removed. For 
example, an extension adds a node to the "Writer  Module" and the 
next office update removes the "Writer Module" (which is rather inconceivable). 
Then the node and its leaves MUST NOT be displayed in OOo's options dialog, 
because the Module is not known. However, it can be displayed in the 
options dialog of the Extension Manager. See also the description for leaf2.oxt.
================================================================================

modules1.oxt: Defines two Modules and three Nodes. The Nodes may not 
be displayed in OOo's options dialog because there is currently no application 
which uses this Module. However the Nodes are displayed in the options dialog 
of the Extension Manager.
There are three Nodes defined. The relation ship is this:

-module1
	-node 1
	      -leaf 1
	      -leaf 2
	      -leaf 3
 	-node 2
	      -leaf 1
	      -leaf 2
	      -leaf 3
	-node 3
	      -leaf 1
	      -leaf 2
	      -leaf 3

-module2
	-node1
	      -leaf 1
	      -leaf 2
	      -leaf 3
	-node3
	      -leaf 1
	      -leaf 2
	      -leaf 3

The options dialog of the Extension Manager shall display only three nodes:

	-node 1
	      -leaf 1
	      -leaf 2
	      -leaf 3
 	-node 2
	      -leaf 1
	      -leaf 2
	      -leaf 3
	-node 3
	      -leaf 1
	      -leaf 2
	      -leaf 3

or 

	-node 1
	      -leaf 1
	      -leaf 2
	      -leaf 3
 	-node 3
	      -leaf 1
	      -leaf 2
	      -leaf 3
	-node 2
	      -leaf 1
	      -leaf 2
	      -leaf 3

Since the order of Module|s is not defined, the dialog may display first the 
Nodes from module2 and then from module1. If a node is already displayed then 
it is not shown again.

================================================================================

modules2.oxt: Same as modules1, except that the order of nodes and leaves 
is not defined.
