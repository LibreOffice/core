/* global cy expect require */

var helper = require('./helper');

function _checkSelectionStart(value) {
	cy.get('@clipboard').should(($c) => {
		expect($c.get(0)._getSelectionStart()).to.eq(value);
	});
}

function _checkSelectionEnd(value) {
	cy.get('@clipboard').should(($c) => {
		expect($c.get(0)._getSelectionEnd()).to.eq(value);
	});
}

function type(text, times = 1) {
	cy.log('>> type - start');

	var input = '';
	for (var i = 0; i < times; ++i) {
		input += text;
	}

	// type against a settled mirror, not one still catching up on replies
	cy.getFrameWindow().then(function (win) {
		return helper.processToIdle(win);
	});

	cy.get('@clipboard').type(input, {delay: 10, force: true});

	cy.log('<< type - end');
}

function moveCaret(direction, modifier = '', times = 1) {
	cy.log('>> moveCaret - start');
	cy.log('  Param - direction: ' + direction);

	if (modifier)
		cy.log('  Param - modifier: ' + modifier);
	if (times > 1)
		cy.log('  Param - times: ' + times);

	var dirKey = '';
	if (direction === 'up') {
		dirKey = '{uparrow}';
	} else if (direction === 'down') {
		dirKey = '{downarrow}';
	} else if (direction === 'left') {
		dirKey = '{leftarrow}';
	} else if (direction === 'right') {
		dirKey = '{rightarrow}';
	} else if (direction === 'home') {
		dirKey = '{home}';
	} else if (direction === 'end') {
		dirKey = '{end}';
	} else if (direction === 'pgup') {
		dirKey = '{pageUp}';
	} else if (direction === 'pgdown') {
		dirKey = '{pageDown}';
	}

	var key = '';
	for (var i = 0; i < times; ++i) {
		key += dirKey;
	}
	if (modifier === 'ctrl') {
		key = '{ctrl}' + key;
	} else if (modifier === 'shift') {
		key = '{shift}' + key;
	}

	cy.get('@clipboard').type(key, {delay: 10, force: true});

	cy.log('<< moveCaret - end');
}

function select(start, end) {
	cy.log('>> select - start');

	moveCaret('home');
	moveCaret('right', '', start);
	if (start < end) {
		moveCaret('right', 'shift', end - start);
	}
	else if (start > end) {
		moveCaret('left', 'shift', start - end);
	}

	cy.log('<< select - end');
}

function checkHTMLContent(content) {
	cy.log('>> checkHTMLContent - start');

	cy.get('@clipboard').should(($c) => {
		expect($c).have.html($c.get(0)._wrapContent(content));
	});

	cy.log('<< checkHTMLContent - end');
}

function checkPlainContent(content) {
	cy.log('>> checkPlainContent - start');

	cy.get('@clipboard').should('have.text', content);

	cy.log('<< checkPlainContent - end');
}

function checkSelectionRange(start, end) {
	cy.log('>> checkSelectionRange - start');

	if (start > end) {
		var t = start;
		start = end;
		end = t;
	}
	cy.get('@clipboard').should('have.prop', 'isSelectionNull', false);
	_checkSelectionStart(start);
	_checkSelectionEnd(end);

	cy.log('<< checkSelectionRange - end');
}

function checkCaretPosition(pos) {
	cy.log('>> checkCaretPosition - start');

	_checkSelectionStart(pos);
	_checkSelectionEnd(pos);

	cy.log('<< checkCaretPosition - end');
}

function checkSelectionIsNull() {
	cy.log('>> checkSelectionIsNull - start');

	cy.get('@clipboard').should('have.prop', 'isSelectionNull', true);

	cy.log('<< checkSelectionIsNull - end');
}

function checkSelectionIsEmpty(pos) {
	cy.log('>> checkSelectionIsEmpty - start');

	cy.get('@clipboard').should('have.prop', 'isSelectionNull', false);
	_checkSelectionStart(pos);
	_checkSelectionEnd(pos);

	cy.log('<< checkSelectionIsEmpty - end');
}

module.exports.type = type;
module.exports.moveCaret = moveCaret;
module.exports.select = select;
module.exports.checkHTMLContent = checkHTMLContent;
module.exports.checkPlainContent = checkPlainContent;
module.exports.checkSelectionRange = checkSelectionRange;
module.exports.checkCaretPosition = checkCaretPosition;
module.exports.checkSelectionIsNull = checkSelectionIsNull;
module.exports.checkSelectionIsEmpty = checkSelectionIsEmpty;
